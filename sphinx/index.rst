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
      
.. todo:: Better documentation
.. todo:: Move code from bib_file.h to source file
.. todo:: In clean, remove tildes from author lists
.. todo:: Use the bibtex-spirit writer?
      
Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
