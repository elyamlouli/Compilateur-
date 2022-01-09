
prefixe=decaf
YACC = bison -y

# exige 3 fichiers:
# - $(prefixe).y (fichier bison)
# - $(prefixe).l (fichier flex)
# - $(prefixe).c (programme principal)
# construit un exécutable nommé $(prefixe)


all: $(prefixe)

$(prefixe): $(prefixe).tab.o lex.yy.o $(prefixe).o lib.o
	$(CC) $^ -o $@ -g

$(prefixe).tab.c: $(prefixe).y
	bison -d -v $(prefixe).y

lex.yy.c: $(prefixe).l $(prefixe).tab.h
	flex $(prefixe).l

lib.o: lib.c lib.h

doc:
	bison --report=all --report-file=$(prefixe).output \
		--graph=$(prefixe).dot --output=/dev/null \
		$(prefixe).y
	dot -Tpdf < $(prefixe).dot > $(prefixe).pdf

clean:
	rm -f *.o $(prefixe).tab.c $(prefixe).tab.h lex.yy.c $(prefixe) \
		$(prefixe).output $(prefixe).dot $(prefixe).pdf
