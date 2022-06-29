#pragma once

#include <vector>

namespace worth{ namespace plugins { namespace p2p {

#ifdef IS_TEST_NET
const std::vector< std::string > default_seeds;
#else
const std::vector< std::string > default_seeds = {
   "seed-east.worthit.com:2001",          // worthit
   "seed-central.worthit.com:2001",       // worthit
   "seed-west.worthit.com:2001",          // worthit
   "52.74.152.79:2001",                   // smooth.witness
   "anyx.io:2001",                        // anyx
   "seed.liondani.com:2016",              // liondani
   "gtg.worth.house:2001",                // gtg
   "seed.jesta.us:2001",                  // jesta
   "lafonaworth.com:2001",                // lafona
   "worth-seed.altcap.io:40696",          // ihashfury
   "seed.roelandp.nl:2001",               // roelandp
   "seed.timcliff.com:2001",              // timcliff
   "worthseed.clayop.com:2001",           // clayop
   "seed.worthviz.com:2001",              // ausbitbank
   "worth-seed.lukestokes.info:2001",     // lukestokes
   "seed.worthian.info:2001",             // drakos
   "seed.followbtcnews.com:2001",         // followbtcnews
   "node.mahdiyari.info:2001",            // mahdiyari
   "seed.riverworth.com:2001",            // riverhead
   "seed1.blockbrothers.io:2001",         // blockbrothers
   "worthseed-fin.privex.io:2001",        // privex
   "seed.yabapmatt.com:2001"              // yabapmatt
};
#endif

} } } // worth::plugins::p2p
