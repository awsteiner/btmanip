btmanip : Command-line BibTeX file manipulator
==============================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Of course, there are several other tools which do similar things,
bibdesk, jabref, zotero, etc., but it was easier for me to construct a
tool myself which was a bit easier to set up the way I wanted it.

Based on bibtex-spirit by Sergiu Dotenco at
https://bitbucket.org/sergiu/bibtex-spirit .

This documentation was developed with the
use of `doxygen <http://www.doxygen.org>`_ ,
`sphinx <http://www.sphinx-doc.org>`_ , and
`breathe <https://github.com/michaeljones/breathe>`_ .

Installation
============

``btmanip`` requires that O\ :sub:`2`\ scl (see `link
<http://web.utk.edu/~asteine1/o2scl>`_ ) has been installed (with HDF5
support enabled). You will also have to manually edit the ``makefile``
in the top directory so that the appropriate header files and
libraries can be found.
      
Example
=======

.. include:: static/example.out
   :literal:

Classes
=======
      
.. doxygenclass:: btmanip::btmanip_class
   :members:
   :protected-members:
   :undoc-members:

.. doxygenclass:: btmanip::bib_file
   :members:
   :protected-members:
   :undoc-members:

Todo List
=========
      
.. todo:: Use the bibtex-spirit writer
.. todo:: Move code from bib_file.h to source file

      
Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
