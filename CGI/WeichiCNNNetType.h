#ifndef WEICHICNNNETTYPE_H
#define WEICHICNNNETTYPE_H

enum WeichiCNNNetType {
	CNN_NET_UNKNOWN,
	CNN_NET,
	CNN_NET_SL,
	CNN_NET_RL,
	CNN_NET_BV_VN,

	CNN_NET_SIZE		// total size, add new element before this one
};

inline std::string getWeichiCNNNetTypeString( WeichiCNNNetType type )
{
	switch( type ) {
	case CNN_NET_UNKNOWN:	return "Unknown";
	case CNN_NET:			return "NET";
	case CNN_NET_SL:		return "SL";
	case CNN_NET_RL:		return "RL";
	case CNN_NET_BV_VN:		return "BV_VN";
	default:
		//should not happen
		assert( false );
		return "error net type!";
	}
}

inline WeichiCNNNetType getWeichiCNNNetType( std::string sFeatureName )
{
	for( int type=0; type<CNN_NET_SIZE; type++ ) {
		if( sFeatureName!=getWeichiCNNNetTypeString(static_cast<WeichiCNNNetType>(type)) ) { continue; }
		return static_cast<WeichiCNNNetType>(type);
	}

	return CNN_NET_UNKNOWN;
}

#endif