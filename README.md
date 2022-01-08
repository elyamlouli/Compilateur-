# projet-compilation

moins prioritaire :
  - overflow dans MIPS ?
  - meilleur affichage erreur

reste a faire :
  - operateur
  - affectation - redirection tableau + var glob
  - fonction declaration et appel (aussi recursif), enrengistrer registre retour
  - branchement / boucle (label + dans quad ?)

encore:
  - 


class Test {
    void main () {
        int a, b, c;
        a = 1;
        b = 2;
        c = a + b;
    }
}

class Test {
    void main () {
        if (1 < 2) {

        }
    }
}

class VariablesScalaire {

    void main ()
    {
        int gi;
        boolean gb;
        int gi1, gi2, gi3, gi4, gi5;
        boolean gb1, gb2, gb3, gb4, gb5;
    }

}

class IfSansElse {

    void main ()
    {
        if (1 == 1) {}
    }

}

class GV {
    int a;
    boolean b;
    int c [10];
    void main() {
        WriteString("string1");
    }
}
