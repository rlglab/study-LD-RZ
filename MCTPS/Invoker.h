#ifndef INVOKER_H
#define INVOKER_H

#include <sstream>
#include <string>
#include <limits>
using std::string ;

/*!
    @brief  base class of Invoker
    @author T.F. Liao
*/
class BaseInvoker
{
public:
    /*!
        @brief  prototype of functor that load value
        @author T.F. Liao
        @param  key [in] key of variable set
        @param  value [in] value of variable set
        @return load successful or not
        in common case, key is not necessary, but there may be some situation need it
        so we keep this field
    */
    virtual bool operator()(const string& key, const string& value) = 0;

	/*!
		@brief	virtual destructor for inherit
	*/
	virtual ~BaseInvoker() {}
};

/*!
    @brief  template implementation with storage type and loader
    @author T.F. Liao
*/
template<class T, class _Loader>
class Invoker : public BaseInvoker
{
private:
    T& m_ref;
    _Loader m_loader ;
public:
    /*!
        @brief  constructor with reference to storage and loader
        @author T.F. Liao
        @param  value [in] reference to storage
        @param  loader [in] loader to handle the type of variable
    */
    Invoker ( T& value, _Loader loader ) : m_ref(value), m_loader(loader) {}

    /*!
        @brief  functor that load value
        @author T.F. Liao
        @param  key [in] key of variable set
        @param  value [in] value of variable set
        @return load successful or not
    */
    bool operator()(const string& key, const string& value) { return m_loader (m_ref, key, value); }
};


/*!
    @breif  a template implementation of loader
    @author T.F. Liao
    @param  ref [out]   loaded value to store in
    @param  key [in] key of variable set
    @param  value [in] value of variable set
    @return load successful or not
    the implementation is convert string to the type with stringstream
    and check if any chars not used.
*/
template<class T>
bool load ( T& ref, const string& key, const string& value )
{
    std::istringstream iss(value);
    if ( std::numeric_limits<T>::is_integer ) {
        string prefix = value.substr(0,2);
        if ( prefix == "0x" ) { // hex
            iss.ignore(2) ;
            iss >> std::hex ;
        } else if ( value.length() > 1 && prefix[0] == '0' ) { // oct
            iss.ignore(1) ;
            iss >> std::oct ;
        }
    }

    iss >> ref ;

    if ( !iss || iss.rdbuf()->in_avail() != 0 )  
        return false;
    return true;
}
/*!
    @brief  specialization implementation of string type
    @author T.F. Liao
    copy the value to ref
*/
template<>
bool load<string> ( string& ref, const string& key, const string& value ) ;

/*!
    @brief  specialization implementation of boolean type
    @author T.F. Liao
    ref = true if value is neither "true" (case insensitive) or "1"
*/
template<>
bool load<bool> ( bool& ref, const string& key, const string& value ) ;

/*!
    @brief  special implementation of loader
    @author T.F. Liao
    construct with a value, once invoked, set the value in ref
*/
template<class T> 
class ValueSetter
{
    T m_value ;
public:
    ValueSetter ( const T& value ) : m_value(value) {}
    bool operator()(T& ref, const string& key, const string& value) { ref = m_value ; return true ; }
};



#endif 
