\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{spec}

\ExecuteOptions{11pt,a4paper}
\ProcessOptions

\LoadClass{article}

% UTF-8 input
\RequirePackage{ucs}
\RequirePackage[utf8x]{inputenc}
\RequirePackage[english]{babel}
\RequirePackage[margin=3cm]{geometry}
\RequirePackage{amsmath}
\RequirePackage{amssymb}
\RequirePackage{latexsym}
\RequirePackage{amsxtra}
\RequirePackage{amscd}
\RequirePackage{charter}
\RequirePackage{fancyhdr}
\RequirePackage{color}
\RequirePackage{hyperref}
\RequirePackage{url}
\pagestyle{plain}

% dont indent paragraphs
\setlength{\parindent}{0pt}
\setlength{\parskip}{0pt}

% some highlighting
% \todo (no args)
\definecolor{red}{rgb}{1,0,0}
\newcommand{\todo}{\textbf\textsf{\color{red}TODO: }}
% \ann{text...}
\definecolor{brown}{rgb}{0.66,0.33,0}
\newcommand{\ann}[1]{\textit\textsf{\color{brown}#1}}

% RFC2119, use like "x \should\ blah"
% (dont forget the \ at the end or it will stick to the next word)
\newcommand{\may}{\textsc{may}}
\newcommand{\should}{\textsc{should}}
\newcommand{\shouldnot}{\textsc{should not}}
\newcommand{\must}{\textsc{must}}
\newcommand{\mustnot}{\textsc{must not}}

% oc, as in override code - use: \oc{fade} (no \ inside!)
\newcommand{\oc}[1]{\texttt{\textbackslash #1}}

\newcommand{\spectitle}{
	\maketitle
	\setlength{\parskip}{0pt}
	\tableofcontents
	\setlength{\parskip}{8pt}
	\newpage
}

\endinput
