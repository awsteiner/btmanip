btmanip : Command-line BibTeX file manipulator
==============================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

``btmanip`` is a tool for managing and processing BibTeX files.
Of course, there are several other tools which do similar things,
bibdesk, jabref, zotero, etc., but it was easier for me to construct a
tool myself which was a bit easier to set up the way I wanted it.
The BibTeX parser is built upon
`bibtex-spirit <https://bitbucket.org/sergiu/bibtex-spirit>`_
by Sergiu Dotenco. 

This documentation was developed with the
use of `doxygen <http://www.doxygen.org>`_ ,
`sphinx <http://www.sphinx-doc.org>`_ , and
`breathe <https://github.com/michaeljones/breathe>`_ .

Installation
============

The full source distribution can be obtained
from `github <https://github.com/awsteiner/btmanip>`_ .
``btmanip`` requires that O\ :sub:`2`\ scl (see `link
<http://web.utk.edu/~asteine1/o2scl>`_ ) has been installed (with HDF5
support enabled). You will also have to manually edit the ``makefile``
in the top directory so that the appropriate header files and
libraries can be found. On OSX, you can use homebrew, so
``brew install --HEAD awsteiner/science/btmanip`` may work.
      
Basic Operation
===============

Typical usage
-------------

Read a .bib file, look for duplicates, and overwrite:

``btmanip -parse my.bib -dup -bib my.bib``

Read a bib file and output in a format useful for an
LaTeX NSF bio sketch (see :cpp:func:`btmanip::btmanip_class::nsf` )

``btmanip -parse my.bib -nsf refs.txt``

Get help on the ``parse`` command:

``btmanip -h parse``

List all of the commands:

``btmanip --commands``

Read a .bib file, in entry Smith75, set the month field to April,
then output to a new file:

``btmanip -parse my.bib -set-field Smith75 month Apr -bib out.bib``

Read a .bib file, list all the keys which begin with the
letter S, and get the entry which starts with Smith

``btmanip -parse my.bib -list-keys S* -get-key Smith*``

Read a .bib file, add a second .bib file, clean, sort, and
save in a new file:

``btmanip -parse one.bib -add two.bib -clean -sort -bib all.bib``

Default commands
----------------

If an environment variable ``BTMANIP_DEFAULTS`` is present, then
it is assumed that it contins a list of commands to be run
each time ``btmanip`` is run.

Readline history
----------------

``btmanip`` uses the environment variable ``HOME`` to put the readline
history file ``.btmanip_hist``. If this environment variable does not
exist (or is empty), then the readline history is not stored or used.

Long Example
============

.. include:: static/example.out
   :literal:

Classes
=======

Class btmanip_class
-------------------

.. doxygenclass:: btmanip::btmanip_class
   :members:
   :protected-members:
   :undoc-members:

Class bib_file
-------------------

.. doxygenclass:: btmanip::bib_file
   :members:
   :protected-members:
   :undoc-members:

Todo List
=========

.. todo:: Better documentation.
.. todo:: A mechanism for removing curly braces and other 'extra'
          characters from author and title entries for text mode. (How
          do we distinguish necessary from unnecessary curly braces?)
          See discussion in parse_author() docs.
.. todo:: Move code from bib_file.h to a new source file bib_file.cpp
.. todo:: In clean, remove tildes from author lists, and rework loop
	  to operate one entry at a time.
.. todo:: Use the bibtex-spirit writer?
.. todo:: Unicode support?	  
.. todo:: The code seems to crash if there are errant backslashes in
	  the journal name. Fix this.
      
Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
