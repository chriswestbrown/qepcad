####################################################################
## SLFQ: Simplifying Large Formulas with QEPCAD
####################################################################
CC = g++
flagso = -O4
flagsd = -g


INCLUDES  =	-I${saclib}/include \
		-I${qe}/source \
		-I.

EXTLIBS   =	${qe}/source/qepcad.a \
		${qe}/extensions/sfext/sfexto.a \
		${qe}/extensions/lift2D/lift2Do.a \
		${qe}/extensions/adj2d/adj2do.a \
		${qe}/extensions/rend/rendo.a \
		${qe}/extensions/sfext/sfexto.a \
		${qe}/source/qepcad.a \
		${saclib}/lib/saclibo.a -lcurses -lreadline


EXTLIBSD  =	${qe}/source/qepcadd.a \
		${qe}/extensions/sfext/sfextd.a \
		${qe}/extensions/lift2D/lift2Dd.a \
		${qe}/extensions/adj2d/adj2dd.a \
		${qe}/extensions/rend/rendd.a \
		${qe}/extensions/sfext/sfextd.a \
		${qe}/source/qepcadd.a \
		${saclib}/lib/saclibd.a -lcurses -lreadline

opt:	std
	

std:
	bison -d -v poly.y
	flex poly.l
	${CC} ${flagso} ${INCLUDES} -o slfq main.cc utils.cc simplify.cc slfq_signal.cc \
		lex.yy.c poly.tab.c \
		${qe}/source/saclib/GCSI.c ${qe}/source/saclib/FAIL.c \
		${EXTLIBS}

deb:
	bison -d -v poly.y
	flex poly.l
	${CC} ${flagsd}  ${INCLUDES} -o slfqd main.cc utils.cc simplify.cc slfq_signal.cc \
		lex.yy.c poly.tab.c \
		${qe}/source/saclib/GCSI.c ${qe}/source/saclib/FAIL.c \
		${EXTLIBSD} #-lefence

parse:	
	bison -d -v poly.y
	flex poly.l
	${CC} ${flagso} -o parself main_parse.cc lex.yy.c poly.tab.c
