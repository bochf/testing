
# No interpreter specified - Because we should be sourcing this file.

# =============================================================================
# Name: system_is_lop
# Description: Determines if we are running on a LoP system
# Paramaters: None
# Returns: shell true (0) if on a LoP system, shell false (1) if not
# Side effects: None
# Notes:
#   Call it thusly:
#
#    if system_is_lop
#    then
#       echo "Is LoP"
#    else
#       echo "Is not LoP"
#    fi
#
#   -- or --
#
#   if ! system_is_lop
#   then
#      echo "ERROR: This is not a Linux on Power system!"
#   fi
#
system_is_lop ()
{
    if [[ $(uname -s) != "Linux" ]]
    then
        return 1
    fi

    if [[ $(uname -p) != "ppc64" ]]
    then
        return 1
    fi

    return 0
}
