
/***************************************************************************
 * NpingTargets.cc -- Class that handles target spec parsing and allows to *
 * obtain the different targets that need to be ping-ed.                   *
 *                                                                         *
 ***********************IMPORTANT NMAP LICENSE TERMS************************
 *                                                                         *
 * The Nmap Security Scanner is (C) 1996-2012 Insecure.Com LLC. Nmap is    *
 * also a registered trademark of Insecure.Com LLC.  This program is free  *
 * software; you may redistribute and/or modify it under the terms of the  *
 * GNU General Public License as published by the Free Software            *
 * Foundation; Version 2 with the clarifications and exceptions described  *
 * below.  This guarantees your right to use, modify, and redistribute     *
 * this software under certain conditions.  If you wish to embed Nmap      *
 * technology into proprietary software, we sell alternative licenses      *
 * (contact sales@insecure.com).  Dozens of software vendors already       *
 * license Nmap technology such as host discovery, port scanning, OS       *
 * detection, version detection, and the Nmap Scripting Engine.            *
 *                                                                         *
 * Note that the GPL places important restrictions on "derived works", yet *
 * it does not provide a detailed definition of that term.  To avoid       *
 * misunderstandings, we interpret that term as broadly as copyright law   *
 * allows.  For example, we consider an application to constitute a        *
 * "derivative work" for the purpose of this license if it does any of the *
 * following:                                                              *
 * o Integrates source code from Nmap                                      *
 * o Reads or includes Nmap copyrighted data files, such as                *
 *   nmap-os-db or nmap-service-probes.                                    *
 * o Executes Nmap and parses the results (as opposed to typical shell or  *
 *   execution-menu apps, which simply display raw Nmap output and so are  *
 *   not derivative works.)                                                *
 * o Integrates/includes/aggregates Nmap into a proprietary executable     *
 *   installer, such as those produced by InstallShield.                   *
 * o Links to a library or executes a program that does any of the above   *
 *                                                                         *
 * The term "Nmap" should be taken to also include any portions or derived *
 * works of Nmap, as well as other software we distribute under this       *
 * license such as Zenmap, Ncat, and Nping.  This list is not exclusive,   *
 * but is meant to clarify our interpretation of derived works with some   *
 * common examples.  Our interpretation applies only to Nmap--we don't     *
 * speak for other people's GPL works.                                     *
 *                                                                         *
 * If you have any questions about the GPL licensing restrictions on using *
 * Nmap in non-GPL works, we would be happy to help.  As mentioned above,  *
 * we also offer alternative license to integrate Nmap into proprietary    *
 * applications and appliances.  These contracts have been sold to dozens  *
 * of software vendors, and generally include a perpetual license as well  *
 * as providing for priority support and updates.  They also fund the      *
 * continued development of Nmap.  Please email sales@insecure.com for     *
 * further information.                                                    *
 *                                                                         *
 * As a special exception to the GPL terms, Insecure.Com LLC grants        *
 * permission to link the code of this program with any version of the     *
 * OpenSSL library which is distributed under a license identical to that  *
 * listed in the included docs/licenses/OpenSSL.txt file, and distribute   *
 * linked combinations including the two. You must obey the GNU GPL in all *
 * respects for all of the code used other than OpenSSL.  If you modify    *
 * this file, you may extend this exception to your version of the file,   *
 * but you are not obligated to do so.                                     *
 *                                                                         *
 * If you received these files with a written license agreement or         *
 * contract stating terms other than the terms above, then that            *
 * alternative license agreement takes precedence over these comments.     *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes (none     *
 * have been found so far).                                                *
 *                                                                         *
 * Source code also allows you to port Nmap to new platforms, fix bugs,    *
 * and add new features.  You are highly encouraged to send your changes   *
 * to nmap-dev@insecure.org for possible incorporation into the main       *
 * distribution.  By sending these changes to Fyodor or one of the         *
 * Insecure.Org development mailing lists, or checking them into the Nmap  *
 * source code repository, it is understood (unless you specify otherwise) *
 * that you are offering the Nmap Project (Insecure.Com LLC) the           *
 * unlimited, non-exclusive right to reuse, modify, and relicense the      *
 * code.  Nmap will always be available Open Source, but this is important *
 * because the inability to relicense code has caused devastating problems *
 * for other Free Software projects (such as KDE and NASM).  We also       *
 * occasionally relicense the code to third parties as discussed above.    *
 * If you wish to specify special license conditions of your               *
 * contributions, just say so when you send them.                          *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License v2.0 for more details at                         *
 * http://www.gnu.org/licenses/gpl-2.0.html , or in the COPYING file       *
 * included with Nmap.                                                     *
 *                                                                         *
 ***************************************************************************/

#include "nping.h"
#include "NpingOps.h"
#include "global_structures.h"
#include "output.h"
#include "nbase.h"
#include "utils.h"
#include "NpingTargets.h"
#include "common.h"
#include "common_modified.h"

extern NpingOps o;

NpingTargets::NpingTargets(){
  memset(specs, 0, 1024*(sizeof(char *)) );
  memset(skipspec, 0, 1024*(sizeof(bool)) );
  speccount=0;
  current_spec=-1;
  finished=false;
  targets_fetched=0;
  current_target=0;
  ready=false;
} /* End of NpingTargets constructor */


NpingTargets::~NpingTargets(){
} /* End of NpingTargets destructor */


/** Adds a target specification to an internal array of specs */
int NpingTargets::addSpec(char *spec){
  if(spec==NULL)
    return OP_FAILURE;
  if( this->speccount >= 1024 )
    return OP_FAILURE;
  specs[ this->speccount ] = spec;
  this->speccount++;
  return OP_SUCCESS;
} /* End of NpingTargets */


/** Returns next target */
int NpingTargets::getNextTargetAddressAndName(struct sockaddr_storage *t, size_t *tlen, char *hname, size_t hlen){
  struct sockaddr_storage next;
  memset(&next, 0, sizeof(struct sockaddr_storage));
  size_t nextlen=0;
  int r=0;
  int family= (o.getIPVersion()==IP_VERSION_6) ? AF_INET6 : AF_INET;

  if( t==NULL || tlen==NULL )
    outFatal(QT_3,"getNextTarget(): NULL values supplied.");

  /* Return failure if there are no specs or we noticed that we were finished in
   * a previous call. */
  if ( this->speccount <= 0 || finished==true )
    return OP_FAILURE;

  /* If this is the first time we call to this method */
  if (this->current_spec == -1 ){

    current_spec=0;
    if ( !skipspec[ current_spec ] ){
     if ( current_group.parse_expr( specs[ current_spec ], family ) != 0 ){
        skipspec[ current_spec ]=true; /* Make sure we skip it next time */
        return OP_FAILURE;

        }
    }
    else{ /* We are skipping current target, return the next one */
        return  this->getNextTargetAddressAndName(t, tlen, hname, hlen);
    }
  }

  r=current_group.get_next_host(&next, &nextlen);

  if ( r!=0 ){ /* We exhausted current group */
    /* Is there any other group? */
     if (++current_spec == speccount){ /* No more specs to parse */
        finished=true;
        return OP_FAILURE;
     }
     /* Ok, there are more groups, so let's go with the next spec */
    if ( !skipspec[ current_spec ] ){
        if ( current_group.parse_expr( specs[ current_spec ], family ) != 0 ){
            skipspec[ current_spec ]=true;
            return this->getNextTargetAddressAndName(t, tlen, hname, hlen);

        }
    }
    else{ /* We are skipping current target, return the next one */
        return  this->getNextTargetAddressAndName(t, tlen, hname, hlen);
    }

    r=current_group.get_next_host(&next, &nextlen);

     if (r != 0)
        outFatal(QT_3,"BUG: TargetGroups are supposed to contain at least one IP! ");
  }
  memcpy( t, &next, sizeof( struct sockaddr_storage ) );
  /* If current spec is a named host (not a range), store name in supplied buff */
  if(current_group.get_namedhost()){
    if( hname!=NULL && hlen>0 )
        strncpy(hname, specs[ current_spec ], hlen);  
  }else{ /* If current spec is not a named host, insert NULL in the first position */
    if( hname!=NULL && hlen>0 )
        hname[0]='\0';
  } 
  *tlen=nextlen;
  targets_fetched++;
  return OP_SUCCESS;
 } /* End of getNextTarget() */


int NpingTargets::getNextIPv4Address(u32 *addr){
  struct sockaddr_storage t;
  size_t tlen;
  char buff[257];
  memset(buff, 0, 257);
  if( addr == NULL )
    outFatal(QT_3, "getNextIPv4Address(): NULL value supplied. ");
  if ( this->getNextTargetAddressAndName(&t, &tlen, buff, 256) != OP_SUCCESS )
    return OP_FAILURE;
  struct sockaddr_in *p=( struct sockaddr_in *)&t;
  if(p->sin_family!=AF_INET)
	outFatal(QT_3, "getNextIPv4Address(): Trying to obtain an IPv4 address from an IPv6 target.");
  *addr = p->sin_addr.s_addr;
  return OP_SUCCESS;
} /* End of getNextIPv4Address() */


int NpingTargets::rewindSpecs(){
  current_spec=-1;
  finished=false;
  targets_fetched=0;
  return OP_SUCCESS;
} /* End of rewind() */


unsigned long int NpingTargets::getTargetsFetched(){
  return this->Targets.size();
} /* getTargetsFetched() */


int NpingTargets::getTargetSpecCount(){
  return this->speccount;
} /* End of getTargetSpecCount() */


/** This method should be called when all the target specs have been entered
  * using addSpec(). What it does is to create a NpingTarget objects for
  * each IP address extracted from the specs. Objects are stored in an internal
  * vector. */
int NpingTargets::processSpecs(){
  char buff[MAX_NPING_HOSTNAME_LEN+1];
  struct sockaddr_storage ss;
  size_t slen=0;
  bool result=false;
  struct route_nfo rnfo;
      
  memset(&ss, 0, sizeof(struct sockaddr_storage));
  memset(buff, 0, MAX_NPING_HOSTNAME_LEN+1);

  /* Rewind spec index just in case someone has been playing around with it */
  o.targets.rewindSpecs();

  /* Get next host IP address and, if it is a named host, its hostname */
  while ( this->getNextTargetAddressAndName(&ss, &slen, buff, MAX_NPING_HOSTNAME_LEN) == OP_SUCCESS ){
      NpingTarget *mytarget = new NpingTarget();
      mytarget->setTargetSockAddr(&ss, slen);
      if( buff[0]=='\0')
        mytarget->setNamedHost(false);
      else{
        mytarget->setSuppliedHostName(buff);
        mytarget->setNamedHost(true);
      }

    /* For the moment, we only run this code if we are not dealing with IPv6 */
    if( !o.ipv6() ){

      /* Get all the information needed to send packets to this target.
	   * (Only in case we are not in unprivileged modes) */
		if(o.getMode()!=TCP_CONNECT && o.getMode()!=UDP_UNPRIV){
		  result=route_dst( &ss, &rnfo, o.getDevice(), NULL );
		  if(result==false){
			outError(QT_2, "Failed to determine route to host %s. Skipping it...", mytarget->getTargetIPstr() );
			delete mytarget;
			continue;
		  }

		   /* Determine next hop */
		  if( rnfo.direct_connect ){
			mytarget->setDirectlyConnected(true);
			mytarget->setNextHop(&ss, slen);
		  }
		  else{
			mytarget->setDirectlyConnected(false);  
			mytarget->setNextHop(&rnfo.nexthop, sizeof(struct sockaddr_storage));
		  }
		  /* Source IP address that we should use when targeting this host */
		  mytarget->setSourceSockAddr(&rnfo.srcaddr, sizeof(struct sockaddr_storage));

		  /* If user requested to spoof IP source address, set it */
		  if( o.spoofSource() ){
			mytarget->setSpoofedSourceSockAddr( o.getSourceSockAddr(), sizeof(struct sockaddr_storage));
		  }
		  
		  /* Network interface */  
		  mytarget->setDeviceNames( rnfo.ii.devname, rnfo.ii.devfullname );
		  mytarget->setDeviceType( rnfo.ii.device_type );

		  /* Set source MAC address */
		  mytarget->setSrcMACAddress( rnfo.ii.mac );

		  if( rnfo.ii.device_up == false )
			outError(QT_2, "Device used for target host %s seems to be down.", mytarget->getTargetIPstr());

		  /* Determine next hop MAC address and target MAC address */
		  if( o.sendEth() ){
		    mytarget->determineNextHopMACAddress();
		    mytarget->determineTargetMACAddress(); /* Sets Target MAC only if is directly connected to us */
          }
		  /* If we are in debug mode print target details */
		  if(o.getDebugging() >= DBG_3)
			mytarget->printTargetDetails();
		}
    }else{
        struct sockaddr_storage ss;
        struct sockaddr_in6 *s6=(struct sockaddr_in6 *)&ss;
        memset(&ss, 0, sizeof(sockaddr_storage));
        s6->sin6_family=AF_INET6;        
        mytarget->setSourceSockAddr(&ss, sizeof(struct sockaddr_storage));
    }
    
      /* Insert current target into targets array */
      this->Targets.push_back(mytarget);
  }

  /* getNextTarget() checks this to ensure user has previously called processSpecs() */
  this->ready=true;
  o.targets.rewind();
  return OP_SUCCESS;
} /* End of getTargetSpecCount() */



NpingTarget *NpingTargets::getNextTarget(){
  /* When !ready it means that processSpecs() has not been called yet. This
   * happens when user hits CTRL-C early. */
  if( this->ready == false )
      return NULL;
  /* Did we reach the end of the vector in the last call? */
  if( this->current_target >= this->Targets.size() )
    return NULL;
  outPrint(DBG_4, "Next target returned by getNextTarget(): Targets[%lu/%lu] --> %s \n", this->current_target, (unsigned long) this->Targets.size(), this->Targets.at(this->current_target)->getTargetIPstr() );
  return this->Targets.at( this->current_target++ );
} /* End of getNextTarget() */


int NpingTargets::rewind(){
  current_target=0;
  return OP_SUCCESS;
} /* End of rewind() */

/* Frees all of the Targets. Returns the number of freed targets. The number 
 * return should match value returned by getTargetsFetched().  */
unsigned long int NpingTargets::freeTargets(){
  unsigned long int cnt=0;
  while(!this->Targets.empty()) {
    this->currenths = Targets.back();
    delete currenths;
    Targets.pop_back();
    cnt++;
  }
  return cnt;
} /* End of freeTargets() */


NpingTarget *NpingTargets::findTarget(struct sockaddr_storage *tt){
  size_t i=0;
  struct sockaddr_storage ss;
  size_t ss_len;
  struct sockaddr_in *s_ip4=(struct sockaddr_in *)&ss;
  struct sockaddr_in6 *s_ip6=(struct sockaddr_in6 *)&ss;
  struct sockaddr_in *t_ip4=(struct sockaddr_in *)tt;
  struct sockaddr_in6 *t_ip6=(struct sockaddr_in6 *)tt;

  if (tt==NULL)
    return NULL;

  for(i=0; i<this->Targets.size(); i++){
    this->Targets[i]->getTargetSockAddr(&ss, &ss_len);
    /* Are we are dealing with IPv4 addresses? */
    if( s_ip4->sin_family==AF_INET && t_ip4->sin_family==AF_INET){

        if( !memcmp(&(s_ip4->sin_addr), &(t_ip4->sin_addr), sizeof(struct in_addr)) )
            return this->Targets[i];
    }
    /* Are they IPv6 addresses? */
    else if( s_ip6->sin6_family==AF_INET6 && t_ip6->sin6_family==AF_INET6 ){
        if( !memcmp(&(s_ip6->sin6_addr), &(t_ip6->sin6_addr), sizeof(struct in6_addr)) )
            return this->Targets[i];
    }
    /* Unknown type of address, skipping... */
    else{
        continue;
    }
  }
  return NULL;
} /* End of findTarget() */
