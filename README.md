# projet-compilation

moins prioritaire :
  - overflow dans MIPS ?
  - meilleur affichage erreur

reste a faire :
  - operateur 
  - affectation - redirection tableau + var glob
  - fonction declaration et appel (aussi recursif)
  - branchement / boucle (label + dans quad ?)
  - verif type et kind

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


class MethodesEtParametres {

    boolean fbb (boolean x) {}
    void fvib (int x, boolean y) {}
    int fibi (boolean x, int y) {}
    int fi7 (int i0, boolean i1, int i2, int i3, boolean i4, boolean i5, int i6) {}
    void fv33 (int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7,
               int i8, int i9, int ia, int ib, int ic, int id, int ie, int iF,
               int j0, int j1, int j2, int j3, int j4, int j5, int j6, int j7,
               int j8, int j9, int ja, int jb, int jc, int jd, int je, int jF,
               int x) {}
    void main () {}
}

class prog {
    int a[10];  
    boolean b1, b2;
    void main() {
        a[0] = 34;
        a[1] = a[0];
        b1 = true;
        b2 = false;
        WriteInt(a[1]);
        WriteString("\n");
        WriteBool(b1);
        WriteString("\n");
        WriteBool(b2);
        WriteString("\n");
    }
}

```
$a2 -> taille idx
$a3 -> taille tableau
_TAB_CHECK_IDX:
    bltz $a2, _TAB_ERR
    bge $a2, $a3 TAB_ERR
    li $v1, 0
    jr $ra
_TAB_ERR:
    li $v0, 4
    la $a0, SYS_MSG1
    b _exit

```

class AppelUnArgument {
    void p (int x) {}
    void main ()
    {
        p (1);
    }
}

class AssociativiteAddSub {

    void main ()
    {
        int r;
        r = 57 - 99 + 42;
        WriteInt (r);
        WriteString ("=0\n");
    }

}