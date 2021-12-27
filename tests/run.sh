#!/bin/bash

#
# Configuration
#

# Nom du compilateur
COMPILER=./../decaf

# Utilisation des entrée et sortie standard
STANDARD_CHANNELS=0

# Simulateur
SPIM=spim
# Nombre de lignes à ignorer en début de sortie du simulateur
SPIM_HELLOLINES=5

# Répertoire contenant les tests
if test $(basename $0) == "tests-run.sh"; then # dév. local uniquement
    TEST_DIR=dts-local
else
    TEST_DIR=$(dirname $0)
fi

# Liste des tests à lancer
LIST_FILE=$TEST_DIR/dts.list

# Arrêt à la première erreur (si =1)
STOP_ERROR=0

# Niveau d'affichage :
# 0 = statistiques finales uniquement
# 1 = tests qui échouent
# 2 = tests qui échouent et sections
# 3 = tous les tests
VERBOSITY=3

# Activation des types de tests
ENABLE_C=1
ENABLE_X=1

#
# Fin de la configuration
#


#
# Traitements des options
#
usage () { # progname
    echo "Usage: $1 [-c <path>] [-s] [-e] [-C] [-X] [-l <path>] [-v <int>]"
    echo "    -c précise le compilateur à utiliser"
    echo "    -s exige que le compilateur utilise les canaux standard"
    echo "    -e provoque l'arrêt à la première erreur"
    echo "    -C effectue seulement les tests d'analyse syntaxique"
    echo "    -X effectue seulement les tests d'exécution"
    echo "    -l utilise une liste personnalisée"
    echo "    -v précise le niveau d'affichage :"
    echo "       0 = statistiques finales uniquement"
    echo "       1 + tests qui échouent"
    echo "       2 + entêtes de section"
    echo "       3 + tests qui réussissent"
    echo "    aucune autre option n'est admise"
    exit 1
}
OPTIONS=$(getopt -q -o c:seCXl:v:Q -- "$@")
if test $? -ne 0; then
    usage $0
fi
eval set -- $OPTIONS
DUMP_CONFIG=0
while true; do
    case $1 in
        "-c") COMPILER="$2"; shift 2 ;;
        "-s") STANDARD_CHANNELS=1; shift ;;
        "-e") STOP_ERROR=1; shift ;;
        "-C") ENABLE_C=1; ENABLE_X=0; shift ;;
        "-X") ENABLE_C=0; ENABLE_X=1; shift ;;
        "-l") LIST_FILE="$2"; shift 2 ;;
        "-v") VERBOSITY=$2; shift 2 ;;
        "-Q") DUMP_CONFIG=1; shift ;; # undoc
        "--") shift; break ;;
    esac
done
if test $# -ne 0; then
    usage $0
fi
if test $DUMP_CONFIG -eq 1; then
    printf "directory: [%s]\n" "$TEST_DIR"
    printf "list:      [%s]\n" "$LIST_FILE"
    printf "compiler:  [%s] (stdin/out=%d)\n" "$COMPILER" $STANDARD_CHANNELS
    printf "selection: [stop=%d,comp=%d,exec=%d]\n" $STOP_ERROR $ENABLE_C $ENABLE_X
    printf "verbosity: [%d]\n" $VERBOSITY
    exit 1
fi
# Fin des options

#
# Test d'existence des fichiers/répertoires
#
if ! test -x $COMPILER || ! command -v $COMPILER > /dev/null; then
    echo "compilateur ($COMPILER) introuvable ou non exécutable"
    exit 1
elif ! test -d "$TEST_DIR"; then
    echo "répertoire ($TEST_DIR) inexistant"
    exit 1
elif ! test -r "$LIST_FILE"; then
    echo "liste ($LIST_FILE) introuvable ou non lisible"
    exit 1
elif ! command -v $SPIM > /dev/null; then
    echo "spim introuvable"
    exit 1
elif ! command -v sed > /dev/null || ! command -v awk > /dev/null; then
    echo "sed et/ou awk nécessaire mais introuvable"
    exit 1
fi
# Fin des tests d'existence

# Couleur (ou pas)
if test -t 1; then
    NORMAL="$(tput sgr0)"
    GREEN="$(tput setaf 2)"
    RED="$(tput setaf 1)"
    BOLD="$(tput bold)"
else
    NORMAL=""
    GREEN=""
    RED=""
fi

# Quelques utilitaires
panic () {
    local msg=$1
    echo "Fichier corrompu ($msg)"
    exit 1
}
print_test () { # name e_c e_x r_c r_x
    local name=$1
    local ec=$2
    local ex=$3
    local rc=$4
    local rx=$5
    local res
    test $ec -eq $rc -a $ex -eq $rx
    res=$?
    if test $res -eq 0 -a $VERBOSITY -ge 3 \
            || test $res -eq 1 -a $VERBOSITY -ge 1; then
        if test $res -eq 0; then
            printf "[%sOK%s]" $BOLD$GREEN $NORMAL
        else
            printf "[%sKO%s]" $BOLD$RED $NORMAL
        fi
        printf " = "
        if test $ec -eq $rc; then
            printf "%sok%s" $GREEN $NORMAL
        else
            printf "%sko%s" $RED $NORMAL
        fi
        printf "/"
        if test $ex -eq 2; then
            printf "%s" "--"
        elif test $ex -eq $rx; then
            printf "%sok%s" $GREEN $NORMAL
        else
            printf "%sko%s" $RED $NORMAL
        fi
        printf " %s\n" $name
    fi
    test $res -eq 0
}

# Actions
do_compile () { # name
    local name=$&
    if test $STANDARD_CHANNELS -eq 1; then
        $COMPILER < "$TEST_DIR/$name.decaf" > "$TEST_DIR/$name.mips" \
                  2> "$TEST_DIR/$name.cerr"
    else
        $COMPILER "$TEST_DIR/$name.decaf" -o "$TEST_DIR/$name.mips" \
                  > "$TEST_DIR/$name.cerr" 2>&1
    fi
}

AWKPROG='
BEGIN { vu = 0; }
/^([[:alnum:]]+)=([[:alnum:]]+)$/ { # lignes xxx=xxx
    ++ vu;
    split ($0, m, /=/);
    if (m[1] != m[2])
        exit (1);
    next;
}
{ exit (1); }
END { if (vu == 0) exit (1); }
'
do_execute () { # name
    $SPIM -file "$TEST_DIR/$name.mips" 2>&1 \
        | sed -e 1,${SPIM_HELLOLINES}d > "$TEST_DIR/$name.xout" 2>&1
    awk "$AWKPROG" "$TEST_DIR/$name.xout" > /dev/null 2>&1
}

# Principal
n_success=0
n_failure=0

while read enabled mode category kind name; do
    if test $kind == "SECT"; then
        enabled_sect=$enabled
        if test $enabled_sect == "+" -a $VERBOSITY -ge 2; then
            printf "==== %s\n" $name
        fi
    elif test $kind == "TEST"; then
        if test $enabled_sect == "+" -a $enabled == "+"; then
            action=${mode:0:1}
            result=${mode:1:1}
            # checks
            if test $action != "C" -a $action != "X"; then
                panic "Fichier corrompu (action)"
            fi
            if test $result -ne 0 -a $result -ne 1; then
                panic "Fichier corrompu (resultat)"
            fi
            # skip?
            if test $action == "C" -a $ENABLE_C -eq 0; then
                continue
            elif test $action == "X" -a $ENABLE_X -eq 0; then
                if test $ENABLE_C -eq 1; then
                    action="C"
                    result=0
                else
                    continue
                fi
            fi
            # expected
            if test $action == "C"; then
                e_comp=$result
                e_exec=2
            else
                e_comp=0
                e_exec=$result
            fi
            # actual
            do_compile $name
            r_comp=$?
            if test $r_comp -eq 0 -a $action == "X"; then
                do_execute $name
                r_exec=$?
            else
                r_exec=2
            fi
            # print out (and count)
            print_test $name $e_comp $e_exec $r_comp $r_exec
            r_test=$?
            if test $r_test -eq 0; then
                n_success=$((n_success + 1))
            else
                n_failure=$((n_failure + 1))
            fi
            if test $r_test -ne 0 -a $STOP_ERROR -ne 0; then
                break;
            fi
        fi
    else
        panic "Fichier corrompu (type)"
    fi
done < "$LIST_FILE"
# can't use cat ... | while because it creates a subshell

printf "%d succès %d échecs\n" $n_success $n_failure
