for /r %%i in (*.tex) do (
  pdflatex %%i
  pdflatex %%i
)