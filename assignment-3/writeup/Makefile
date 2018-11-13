# this is a comment

TARGET=assignment-3


default: pdf

bib: bib
	bibtex ${TARGET}

dvi: ${TARGET}.tex
#	run latex twice to get references correct
	# ./latex-git-log --lang=en > table.tex
	latex ${TARGET}.tex
#	you can also have a bibtex line here
#	bibtex $(TARGET)
	latex $(TARGET).tex

ps: dvi
	dvips -R -Poutline -t letter ${TARGET}.dvi -o ${TARGET}.ps

pdf: ps
	ps2pdf ${TARGET}.ps

clean-dvi:
	rm -f ${TARGET}.aux
	rm -f ${TARGET}.log
	rm -f ${TARGET}.dvi
	rm -f ${TARGET}.out
	rm -f ${TARGET}.toc
	rm -f logfile

clean-ps: clean-dvi
	rm -f ${TARGET}.ps

clean-pdf: clean-ps
	rm -f ${TARGET}.bbl
	rm -f ${TARGET}.blg
	rm -f ${TARGET}.pdf
	# rm -f table.tex

clean: clean-pdf
