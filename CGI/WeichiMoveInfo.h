#ifndef _WEICHIMOVEINFO_
#define _WEICHIMOVEINFO_

#include <string>
#include <cstdlib>
#include "WeichiMove.h"
#include "WeichiUctNode.h"
#include "StatisticData.h"
#include "UctChildIterator.h"
#include <sstream>
#include <map>

class WeichiMoveInfo
{
private:
    std::map<WeichiMove, StatisticData> m_info ;
    typedef NodePtr<WeichiUctNode> UctNodePtr ;

    struct Order {
        Order(StatisticData::data_type c=0, std::string s="") : cnt(c), msg(s){}
        StatisticData::data_type cnt ;
        std::string msg;
        bool operator<(const Order& rhs)const {return cnt>rhs.cnt;}
    };
    std::vector<Order> m_list;

public:
    typedef std::map<WeichiMove, StatisticData> container_type ;
    typedef container_type::const_iterator const_iterator ;

public:

    void clear() { m_info.clear(); }

    void parse ( const std::string& s ) {
        std::istringstream iss(s);
        m_info.clear();
        std::string token;
        while ( std::getline(iss, token, ';') ) {
            if ( token.empty() ) continue;
            WeichiMove move ;
            StatisticData data ;
            parse(token, move, data) ;
            m_info[move] = data ;
        }
    }

    std::string serialize ( UctNodePtr node )
    {
        //*
        UctChildIterator<WeichiUctNode> it(node);
        ostringstream oss ;
        for ( ; it ; ++it ) {
            oss << ";" << it->getMove().toSgfString() ;
            oss << "WR[" << it->getUctData().getMean() << "]V[" << it->getUctData().getCount() << "]" ;
        }
        return oss.str();

        /*/
        UctChildIterator<WeichiUctNode> it(node);
        m_list.clear();
        for ( ; it ; ++it ) {
            ostringstream oss ;
            oss << ";" << it->getMove().toSgfString() ;
            oss << "WR[" << it->getUctData().getMean() << "]V[" << it->getUctData().getCount() << "]" ;
            m_list.push_back(Order(it->getUctData().getCount(), oss.str())) ;
        }
        sort(m_list.begin(), m_list.end()) ;
        ostringstream oss ;
        for ( size_t i=0;i<m_list.size();++i ) {
            oss << m_list[i].msg;
        }
        cerr << oss.str().substr(0, 40) << " ... " << endl;

        return oss.str();
        // */
    }

    void merge ( const WeichiMoveInfo& rhs )
    {
        std::map<WeichiMove, StatisticData>::const_iterator it = rhs.m_info.begin();
        for ( ; it != rhs.m_info.end(); ++ it ) {
            m_info[it->first].add(it->second.getMean(), it->second.getCount()) ;
        }
    }

    container_type::const_iterator begin() const { return m_info.begin(); }
    container_type::const_iterator end() const { return m_info.end(); }

private:
    void parse(const std::string& token, WeichiMove& move, StatisticData& data ) 
    {
        float visit = 1.0f, winrate = 0.0f;
        std::istringstream iss(token);
        std::string k, v ;
        
        while ( std::getline(iss, k, '[') ) {
            std::getline(iss, v, ']') ;
            if ( k == "V" ) {
                visit = (float)atof(v.c_str()) ;
            } else if ( k == "WR" ) {
                winrate = (float)atof(v.c_str());
            } else {
                Color c = toColor(k[0]) ;
                move = WeichiMove(c, v) ;
            }
        }
        data.reset(winrate, visit);
    }

};


#endif // _WEICHIMOVEINFO_ 
