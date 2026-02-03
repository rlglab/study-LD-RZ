#ifndef WEICHICNNFEATURETYPE_H
#define WEICHICNNFEATURETYPE_H

enum WeichiCNNFeatureType {
	CNN_FEATURE_UNKNOWN,
	CNN_FEATURE_G49,
	CNN_FEATURE_G50,
	CNN_FEATURE_G51,
	CNN_FEATURE_F16,
	CNN_FEATURE_G17,
	CNN_FEATURE_C49,
	CNN_FEATURE_C51,
	CNN_FEATURE_C52,
	CNN_FEATURE_F18,
	CNN_FEATURE_C148,

	CNN_FEATURE_SIZE		// total size, add new element before this one
};

const int CNN_CHANNEL_SIZE[CNN_FEATURE_SIZE] = {0,49,50,51,16,17,49,51,52,18,148};
//const int MAX_CNN_CHANNEL_SIZE = 51;
const int MAX_CNN_CHANNEL_SIZE = 150;

inline string getWeichiCNNFeatureTypeString( WeichiCNNFeatureType type )
{
	switch( type ) {
	case CNN_FEATURE_UNKNOWN:	return "Unknown";
	case CNN_FEATURE_G49:		return "G49";
	case CNN_FEATURE_G50:		return "G50";
	case CNN_FEATURE_G51:		return "G51";
	case CNN_FEATURE_F16:		return "F16";
	case CNN_FEATURE_G17:		return "G17";
	case CNN_FEATURE_C49:		return "C49";
	case CNN_FEATURE_C51:		return "C51";
	case CNN_FEATURE_C52:		return "C52";
	case CNN_FEATURE_F18:		return "F18";
	case CNN_FEATURE_C148:		return "C148";
	default:
		//should not happen
		assert( false );
		return "error type!";
	}
}

inline WeichiCNNFeatureType getWeichiCNNFeatureType( string sFeatureName )
{
	for( int type=0; type<CNN_FEATURE_SIZE; type++ ) {
		if( sFeatureName!=getWeichiCNNFeatureTypeString(static_cast<WeichiCNNFeatureType>(type)) ) { continue; }
		return static_cast<WeichiCNNFeatureType>(type);
	}

	return CNN_FEATURE_UNKNOWN;
}

#endif