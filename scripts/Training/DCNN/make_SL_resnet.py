#!/usr/bin/env python

from __future__ import print_function
from caffe import layers as L, params as P, to_proto
from caffe.proto import caffe_pb2

def conv_factory(bottom, kernel_size, n_out, stride, pad, name):
    conv = L.Convolution(bottom, kernel_size=kernel_size, stride=stride, num_output=n_out, pad=pad,
                         bias_filler=dict(type='constant', value=0.0), weight_filler=dict(type='msra'), name=name)
    return conv

def residual_block(postfix, bottom, channel):
    con_3x3 = conv_factory(bottom, 3, channel, stride=1, pad=1, name='conv_3x3_%d_%s'%(channel, postfix))
    add = L.Eltwise( bottom, con_3x3, operation=P.Eltwise.SUM, name='eltwise_%s'%(postfix))
    relu = L.ReLU(add, in_place=True, name='relu_%s'%(postfix))
    batch_norm = L.BatchNorm(relu, in_place=True, name='batch_norm_%s'%(postfix),
        param=[
            dict(lr_mult=0, decay_mult=0), 
            dict(lr_mult=0, decay_mult=0), 
            dict(lr_mult=0, decay_mult=0),
        ], 
        batch_norm_param=dict(use_global_stats=False)   # use "False" in training and switch to "True" when testing
    )
    batch_norm = L.Scale(batch_norm, name='scale_%s'%(postfix), bias_term=True, in_place=True)
    return batch_norm

def make_resnet(depth, channel,  classes, batch_size, feature):
    #data, label = L.Data(ntop=2)
    
    data, dummy_data = L.MemoryInputLayer(name="data_input_layer", ntop=2, 
        memory_data_param=dict(
            batch_size=batch_size,      # batch size
            channels=feature,        # feature size
            height=19,
            width=19,
        )
    )

    label, dummay_label = L.MemoryInputLayer(name="label_input_layer", ntop=2, 
        memory_data_param=dict(
            batch_size=batch_size,      # batch size
            channels=classes,       # feature size
            height=1,
            width=1,
        )
    )

    silence = L.SilenceLayer(dummy_data, dummay_label, name='silence_layer')

    # first layer
    con_5x5 = conv_factory(data, 5, channel, stride=1, pad=2, name='conv_5x5_%d_1' % channel)
    con_1x1 = conv_factory(data, 1, channel, stride=1, pad=1, name='conv_1x1_%d_1' % channel)
    add = L.Eltwise(con_5x5, con_1x1, operation=P.Eltwise.SUM)
    output = L.ReLU(add, in_place=True)

    for i in xrange(depth-2):
        output = residual_block(str(i+2), output, channel)

    output = conv_factory(output, 3, 1, stride=1, pad=1, name='conv_3x3_1_%d' % depth)
    flatten = L.Flatten(output, name='flatten_layer')
    probsbility = L.InnerProduct(flatten, num_output=classes, bias_filler=dict(type='constant', value=0.0), weight_filler=dict(type='msra'))
    loss = L.SoftmaxWithLoss(probsbility, label)
    return to_proto(loss, silence)

DEPTH = 13
CHANNEL = 256
CLASSES = 361
BATCH_SIZE = 128
FEATURE = 50

if __name__ == '__main__':
    model_title='T_SL_Go19_resnet'
    model_file='%s.prototxt'%model_title
    with open(model_file, 'w') as f:
        print("Generate prototxt file: %s" % model_file)
        print("  depth:\t%d" % DEPTH)
        print("  channel:\t%d" % CHANNEL)
        print("  classes:\t%d" % CLASSES)
        print("  batch_size:\t%d" % BATCH_SIZE)
        print("  feature:\t%d" % FEATURE)
        print('name: "%s"'%model_title, file=f)
        print(make_resnet(depth=DEPTH, channel=CHANNEL, classes=CLASSES, batch_size=BATCH_SIZE, feature=FEATURE), file=f)
        print("[Warning] You need to modify some blobs' name to match CGI's setting")
