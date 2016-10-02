# btmanip
Command-line BibTeX file manipulator

Of course, there are several other tools which do similar things,
bibdesk, zotero, etc., but it was easier for me to construct a tool
myself which was a bit easier to set up the way I wanted it.

Based on bibtex-spirit by Sergiu Dotenco at
https://bitbucket.org/sergiu/bibtex-spirit .

    $ btmanip
    This program comes with ABSOLUTELY NO WARRANTY; for details
    type `warranty'.  This is free software, and you are welcome
    to redistribute it under certain conditions; type `license'
    for details.
    ------------------------------------------------------------
    btmanip> run examples/example.scr
    # ------------------------------------------------------------
    # This file documents the use of the 'btmanip' command.
    # Read a .bib file using 'parse'
    # ------------------------------------------------------------
    > parse examples/ex1.bib
    Read 65 entries from file examples/ex1.bib
    # ------------------------------------------------------------
    # You can use 'search' to find entries in the .bib file,
    # which removes all entries but the ones that match.
    # ------------------------------------------------------------
    > search and journal "Phys. Rev. Lett." author "*Teaney*"
    1 record found.
    # ------------------------------------------------------------
    # Go back to the original file
    # ------------------------------------------------------------
    > parse examples/ex1.bib
    Read 65 entries from file examples/ex1.bib
    # ------------------------------------------------------------
    # Use 'list-keys' to list all the keys in the current set 
    # of entries
    # ------------------------------------------------------------
    > list-keys
    0. Machleidt:2011zz       22. Molnar:2003ff         44. Shen:1998gq           
    1. Alver:2010gr           23. Teaney:2003kp         45. Akmal:1998cf          
    2. Epelbaum:2008ga        24. Greco:2003xt          46. Bass:1998ca           
    3. Luzum:2008cw           25. Fries:2003vb          47. Kaplan:1998we         
    4. Li:2008gp              26. Maris:2003vk          48. Kaplan:1998tg         
    5. Romatschke:2007mq      27. Danielewicz:2002pu    49. Oset:1997it           
    6. Wicks:2005gt           28. Hirano:2002ds         50. Maris:1997tm          
    7. Andronic:2005yp        29. Bedaque:2002mn        51. Pudliner:1997ck       
    8. Baran:2004ih           30. Teaney:2001av         52. Serot:1997xg          
    9. Lin:2004en             31. Kharzeev:2001gp       53. Lalazissis:1996rd     
    10. Steiner:2004fi        32. Pieper:2001mp         54. Prakash:1996xs        
    11. Epelbaum:2004fk       33. Kharzeev:2000ph       55. Machleidt:1995km      
    12. Gyulassy:2004zy       34. Teaney:2000cw         56. BraunMunzinger:1995bp 
    13. Caurier:2004gf        35. BraunMunzinger:2000px 57. Kaiser:1995eg         
    14. Fries:2003kq          36. Machleidt:2000ge      58. BraunMunzinger:1994xr 
    15. Kolb:2003dz           37. Gyulassy:2000er       59. Wiringa:1994wb        
    16. Rischke:2003mt        38. Roberts:2000aa        60. Stoks:1994wp          
    17. Greco:2003mm          39. Gyulassy:2000fs       61. Gyulassy:1994ew       
    18. Entem:2003ft          40. Hagino:1999xb         62. Moller:1993ed         
    19. BraunMunzinger:2003zd 41. BraunMunzinger:1999qy 63. Gyulassy:1993hr       
    20. Jido:2003cb           42. Drechsel:1998hk       64. Schnedermann:1993ws   
    21. Gyulassy:2003mc       43. Rijken:1998yy         
    # ------------------------------------------------------------
    # 'get-key' returns the entry which matches a specified key
    # ------------------------------------------------------------
    > get-key Epelbaum:2008ga
    @article{Epelbaum:2008ga,
      author =       {Epelbaum, Evgeny and Hammer, Hans-Werner and Meissner,
                            Ulf-G.},
      title =        {Modern Theory of Nuclear Forces},
      journal =      {Rev. Mod. Phys.},
      volume =       81,
      year =         2009,
      pages =        {1773-1825},
      doi =          {10.1103/RevModPhys.81.1773},
      eprint =       {0811.1338},
      archivePrefix ={arXiv},
      primaryClass = {nucl-th},
      reportNumber = {HISKP-TH-08-18, FZJ-IKP-TH-2008-20},
      SLACcitation = {%%CITATION = ARXIV:0811.1338;%%}
    }
    # ------------------------------------------------------------
    # Sort by key
    # ------------------------------------------------------------
    > sort
    # ------------------------------------------------------------
    # Output to a file named 'output.bib'
    # ------------------------------------------------------------
    > bib output.bib
    # ------------------------------------------------------------
    # Shell commands are preceeded by a !
    # ------------------------------------------------------------
    > !head -n 10 output.bib
    : Executing system command: head -n 10 output.bib
    @article{Akmal:1998cf,
      author =       {Akmal, A. and Pandharipande, V. R. and Ravenhall, D. G.},
      title =        {The Equation of state of nucleon matter and neutron star
                            structure},
      journal =      {Phys. Rev.},
      volume =       {C58},
      year =         1998,
      pages =        {1804-1828},
      doi =          {10.1103/PhysRevC.58.1804},
      eprint =       {nucl-th/9804027},
    : Done with system command (returned 0).
    # ------------------------------------------------------------
    # The 'add' command adds entries from a .bib file to the
    # current set of entries, prompting if it finds possible
    # duplicates.
    # ------------------------------------------------------------
    > add examples/ex2.bib
    When adding entry:
    
    @article{Li:2008gp,
      author =       {Li, Bao-An and Chen, Lie-Wen and Ko, Che Ming},
      title =        {Recent Progress and New Challenges in Isospin Physics
                            with Heavy-Ion Reactions},
      journal =      {Phys. Rept.},
      volume =       464,
      year =         2008,
      pages =        {113-281},
      doi =          {10.1016/j.physrep.2008.04.005},
      eprint =       {0804.3580},
      archivePrefix ={arXiv},
      primaryClass = {nucl-th},
      SLACcitation = {%%CITATION = ARXIV:0804.3580;%%}
    }
    
    1 duplicates in the current list were found:
    
    @article{Li:2008gp,
      author =       {Li, Bao-An and Chen, Lie-Wen and Ko, Che Ming},
      title =        {Recent Progress and New Challenges in Isospin Physics
                            with Heavy-Ion Reactions},
      journal =      {Phys. Rept.},
      volume =       464,
      year =         2008,
      pages =        {113-281},
      doi =          {10.1016/j.physrep.2008.04.005},
      eprint =       {0804.3580},
      archivePrefix ={arXiv},
      primaryClass = {nucl-th},
      SLACcitation = {%%CITATION = ARXIV:0804.3580;%%}
    }
    
    Add entry anyway (y/n)? n
    Ignoring Li:2008gp
    When adding entry:
    
    @article{Steiner05ia,
      author =       {Steiner, Andrew W. and Prakash, Madappa and Lattimer,
                            James M. and Ellis, Paul J.},
      title =        {Isospin asymmetry in nuclei and neutron stars},
      journal =      {Phys. Rept.},
      volume =       411,
      year =         2005,
      pages =        {325-375},
      doi =          {10.1016/j.physrep.2005.02.004},
      eprint =       {nucl-th/0410066},
      archivePrefix ={arXiv},
      primaryClass = {nucl-th},
      reportNumber = {LA-UR-04-6745},
      SLACcitation = {%%CITATION = NUCL-TH/0410066;%%}
    }
    
    1 duplicates in the current list were found:
    
    @article{Steiner:2004fi,
      author =       {Steiner, Andrew W. and Prakash, Madappa and Lattimer,
                            James M. and Ellis, Paul J.},
      title =        {Isospin asymmetry in nuclei and neutron stars},
      journal =      {Phys. Rept.},
      volume =       411,
      year =         2005,
      pages =        {325-375},
      doi =          {10.1016/j.physrep.2005.02.004},
      eprint =       {nucl-th/0410066},
      archivePrefix ={arXiv},
      primaryClass = {nucl-th},
      reportNumber = {LA-UR-04-6745},
      SLACcitation = {%%CITATION = NUCL-TH/0410066;%%}
    }
    
    Add entry anyway (y/n)? n
    Ignoring Steiner05ia
    Read 2 entries from file "examples/ex2.bib". Now 65 total entries with 65 sortable entries.
    # ------------------------------------------------------------
    # List the keys for the current set of entries again
    # ------------------------------------------------------------
    > list-keys
    0. Akmal:1998cf           22. Gyulassy:1994ew       44. Maris:2003vk          
    1. Alver:2010gr           23. Gyulassy:2000er       45. Moller:1993ed         
    2. Andronic:2005yp        24. Gyulassy:2000fs       46. Molnar:2003ff         
    3. Baran:2004ih           25. Gyulassy:2003mc       47. Oset:1997it           
    4. Bass:1998ca            26. Gyulassy:2004zy       48. Pieper:2001mp         
    5. Bedaque:2002mn         27. Hagino:1999xb         49. Prakash:1996xs        
    6. BraunMunzinger:1994xr  28. Hirano:2002ds         50. Pudliner:1997ck       
    7. BraunMunzinger:1995bp  29. Jido:2003cb           51. Rijken:1998yy         
    8. BraunMunzinger:1999qy  30. Kaiser:1995eg         52. Rischke:2003mt        
    9. BraunMunzinger:2000px  31. Kaplan:1998tg         53. Roberts:2000aa        
    10. BraunMunzinger:2003zd 32. Kaplan:1998we         54. Romatschke:2007mq     
    11. Caurier:2004gf        33. Kharzeev:2000ph       55. Schnedermann:1993ws   
    12. Danielewicz:2002pu    34. Kharzeev:2001gp       56. Serot:1997xg          
    13. Drechsel:1998hk       35. Kolb:2003dz           57. Shen:1998gq           
    14. Entem:2003ft          36. Lalazissis:1996rd     58. Steiner:2004fi        
    15. Epelbaum:2004fk       37. Li:2008gp             59. Stoks:1994wp          
    16. Epelbaum:2008ga       38. Lin:2004en            60. Teaney:2000cw         
    17. Fries:2003kq          39. Luzum:2008cw          61. Teaney:2001av         
    18. Fries:2003vb          40. Machleidt:1995km      62. Teaney:2003kp         
    19. Greco:2003mm          41. Machleidt:2000ge      63. Wicks:2005gt          
    20. Greco:2003xt          42. Machleidt:2011zz      64. Wiringa:1994wb        
    21. Gyulassy:1993hr       43. Maris:1997tm          
    # ------------------------------------------------------------
    # Sometimes it's helpful to determine which entries in a
    # .bib file are already present in a master .bib file.
    # We first parse the file with additional entries:
    # ------------------------------------------------------------
    > parse examples/ex3.bib
    Read 2 entries from file examples/ex3.bib
    # ------------------------------------------------------------
    # Then use 'sub' to subtract out all entries already present
    # in the master .bib file
    # ------------------------------------------------------------
    > sub examples/ex1.bib
    Read 65 entries from file examples/ex1.bib
    Duplicate keys and duplicate tags: article Bedaque:2002mn
    # ------------------------------------------------------------
    # Now output in .bib format
    # ------------------------------------------------------------
    > bib
    @article{Thorsett:1998uc,
      author =       {Thorsett, S. E. and Chakrabarty, Deepto},
      title =        {Neutron star mass measurements. 1. Radio pulsars},
      journal =      {Astrophys. J.},
      volume =       512,
      year =         1999,
      pages =        288,
      doi =          {10.1086/306742},
      eprint =       {astro-ph/9803260},
      archivePrefix ={arXiv},
      primaryClass = {astro-ph},
      SLACcitation = {%%CITATION = ASTRO-PH/9803260;%%}
    }
    # ------------------------------------------------------------
    # Output in plain text format
    # ------------------------------------------------------------
    > text
    tag: article
    key: Thorsett:1998uc
    author: Thorsett, S. E. and Chakrabarty, Deepto
    author (reformat): S. E. Thorsett and Deepto Chakrabarty
    title: {Neutron star mass measurements. 1. Radio pulsars}
    journal: Astrophys. J.
    volume: 512
    year: 1999
    pages: 288
    doi: 10.1086/306742
    eprint: astro-ph/9803260
    archivePrefix: arXiv
    primaryClass: astro-ph
    SLACcitation: %%CITATION = ASTRO-PH/9803260;%%
    # ------------------------------------------------------------
    # There is a generic help command
    # ------------------------------------------------------------
    > help
    
    List of commands:
    
    add          Add a specified .bib file.                    
    alias        Create a command alias.                       
    bib          Output a .bib file.                           
    clean        Clean.                                        
    commands     List all available commands.                  
    cv           Create output for a CV.                       
    cvt          Create presentations output for a CV.         
    dox          Output a doxygen file for O2scl.              
    dup          Find duplicates between .bib files.           
    exit         Exit (synonymous with 'quit').                
    get          Get the value of a parameter.                 
    get-key      Get entry by key.                             
    hay          Create a simple HTML author-year list.        
    help         Show help information.                        
    keep-field   Remove a field from the remove list.          
    license      Show license information.                     
    list-keys    List entry keys.                              
    nsf          Output LaTeX source for an NSF bio sketch.    
    parse        Parse a specified .bib file.                  
    prop         Output a proposal .bib file.                  
    quit         Quit (synonymous with 'exit').                
    read-jlist   Read a new journal list.                      
    remove-field Add a field to the remove list.               
    run          Run a file containing a list of commands.     
    search       Search.                                       
    set          Set the value of a parameter.                 
    sort         Sort.                                         
    sub          Subtract the current entries from a .bib file.
    text         Output a text file.                           
    warranty     Show warranty information. 
