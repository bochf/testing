#!/bin/ksh

###############################################################################
# Crap-locker
function list_if_devs_forky
{
   # Fork happy
   grep ":" /proc/net/dev | sed 's/:.*//; s/^[[:space:]]*//'
}


#### NOTE: Most of these have been updated/improved in pgm_check
####       Go there first for the latest and best versions
####       Code moved to pgm_chk is denoted with --==MIGRATED==--




###############################################################################
# Stuff that passes


# =============================================================================
#                           --==MIGRATED==--
# Name: list_if_devs   
# Description: Generate a list of (plumbed) interfaces (ifconfig -l)
# Paramaters: none
# Returns: The number of devices found, -1 on error
# Side effects: Writes list of devices to stdout
# Notes: It would be easy, and likely appropriate to filter devices. For
#        example; skip lo and sit0 devices as they are not used.
function list_if_devs
{
   typeset lid_line=
   typeset -i lid_ifcnt=0

   if [[ ! -r /proc/net/dev ]]
   then
       return -1
   fi

   while read lid_line
   do
      if [[ ${lid_line} == *:* ]]
      then
          # Is a device line
          (( lid_ifcnt++ ))

          lid_line=${lid_line%:*}
          echo ${lid_line}
      fi
   done < /proc/net/dev

   return ${lid_ifcnt}
}


# =============================================================================
#                           --==MIGRATED==--
# Name: 
# Description: 
# Paramaters: string - the ethernet device we are looking for
# Returns: 0 on success, non-0 on failure
# Side effects: Writes IP address to stdout
# Notes: It is dangerous to call "ifconfig ethX" and look at the results. The
#        better solution is to check for that device first, *then* call the
#        command. This is because "ifconfig eth7" will plumb eth7 if it was
#        not previously plumbed. (Or so has been my experience on other
#        platforms. We can "fix" this if I am wrong.) The solution here has
#        been to do a single fork, and parse out the results.
function ip_from_eth
{
    typeset ife_ethdev=${1}
    typeset ife_line=
    typeset ife_insec=0

    if [[ -z ${ife_ethdev} ]]
    then
        return 1
    fi

    /sbin/ifconfig | while read ife_line
    do
        if [[ ${ife_line} == ${ife_ethdev}* ]]
        then
            ife_insec=1
        fi

        if [[ ${ife_line} == "" ]]
        then
            ife_insec=0
        fi

        if (( ife_insec == 0 )) ; then continue ; fi

        if [[ ${ife_line} == *"inet addr:"* ]]
        then
            ife_line=${ife_line#*:}
            ife_line=${ife_line%%[[:space:]]*}
            echo ${ife_line}
        fi
    done

    return 0
}



function eth_dev_list
{
    for edl_eth in $(list_if_devs)
    do
        echo ${edl_eth}


    done


}

# =============================================================================
#                           --==MIGRATED==--
# Name: 
# Description: 
# Paramaters: string - the bondX device we are looking at
# Returns: 0 is bond, 1 is not bond, -1 on error
# Side effects: None
# Notes: Call it thusly:
#        if is_bond_if bond0
#        then
#           echo "is a bond"
#        fi
#
function is_bond_if
{
    typeset ibi_bonddev=${1}

    if [[ -z ${ibi_bonddev} ]]
    then
        return -1
    fi
    
    if [[ -e /proc/net/bonding/${ibi_bonddev} ]]
    then
        return 0
    fi

    return 1
}

# =============================================================================
#                           --==MIGRATED==--
# Name: get_bond_legs
# Description: 
# Paramaters: string - the bondX device we are looking at
# Returns: 0 on success, non-0 on error
# Side effects: Writes leg names to stdou
# Notes: The return value is NOT the number of legs. It could be. Currently
#        no legs is an error condition.
function get_bond_legs
{
    typeset gbl_bonddev=${1}
    typeset gbl_legcnt=0

    if [[ -z ${gbl_bonddev} ]]
    then
        return 1
    fi

    if [[ ! -e /proc/net/bonding/${gbl_bonddev} ]]
    then
        return 1
    fi
   
    grep "Slave Interface:" /proc/net/bonding/${gbl_bonddev} | while read gbl_leg
    do
        echo ${gbl_leg#*: }
        (( gbl_legcnt++ ))
    done

    # Consider this a failure
    if ((  gbl_legcnt == 0 )) ; then return 1 ; fi
        
    return 0
}












# -----------------------------------------------------------------------------
# Library testing
echo "__List of interfaces____________________________________________________"
list_if_devs
echo "Got ${?} devices."
echo
echo "__The IP address for eth0_______________________________________________"
ip_from_eth eth0
echo
echo "__Test for bond type____________________________________________________"
if is_bond_if eth0
then
    echo "eth0 is a bond interface"
else
    echo "eth0 is not a bond interface"
fi
if is_bond_if bond0
then
    echo "bond0 is a bond interface"
else
    echo "bond0 is not a bond interface"
fi
if is_bond_if
then
    echo "NULL is a bond interface"
else
    echo "NULL is not a bond interface"
fi
echo
echo "__The IP address for eth0_______________________________________________"
echo "eth0:"
if ! get_bond_legs eth0
then
    echo "eth0 is not a bond interface"
fi
echo "bond0:"
if ! get_bond_legs bond0
then
    echo "bond0 is not a bond interface"
fi
echo "bond1:"
if ! get_bond_legs bond1
then
    echo "bond1 is not a bond interface"
fi
echo "NULL:"
if ! get_bond_legs
then
    echo "NULL is not a bond interface"
fi
echo
