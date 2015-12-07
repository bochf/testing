
# ----------------------------------------------------------------------------
# simple_round
# Rounds a number to 10's
# Requires ksh93 on (at least) AIX
function simple_round
{
    inval=${1}

    if [[ -z ${inval} ]]
    then
        return 1
    fi

    dcount=${#inval}

    #echo ${inval} has ${dcount} digits.

    if (( dcount == 1 ))
    then
        if (( inval >= 5 ))
        then
            echo 10
        else
            echo 0
        fi

        return 0
    fi

    dcount=$(( dcount - 1 ))
    lastdigit=${inval:$dcount}

    #echo last digit is ${lastdigit}

    if (( lastdigit >= 5 ))
    then
        echo $(( inval + 10 - lastdigit ))
    else
        echo $(( inval - lastdigit ))
    fi

    return 0
}
