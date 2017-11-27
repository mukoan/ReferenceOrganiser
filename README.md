# Reference Organiser

For organising technical papers and your notes/reviews on them.

Download binaries for Windows, Mac or Linux from [lyndonhill.com](http://www.lyndonhill.com/Projects/referenceorganiser.html)

Source is under [GNU GPL 3](https://www.gnu.org/licenses/gpl.html).

# Dependencies

You will need [Qt](http://qt.io) version 5.

# Building

* Run qmake on the pro file, e.g. `qmake ReferenceOrganiser.pro -o Makefile` 
* `make`

# Instructions

* Make two directories (if they don't exist already), one for papers, one for reviews
* Copy all your papers to the papers directory
* Rename all paper files to their citation key; e.g. for a paper by A. Smith and B. Jones published in 2017, rename the file to `SmithJones2017.pdf`
* Run ReferenceOrganiser
* Go to Preferences and add the two directories above
* You can create reviews, edit, delete, find paper to match a review, search, etc.

You can use a different citation key format, but you must keep the year at the end. For example, instead of `SmithJones2017.pdf` you could use `SJ_SwabbleBuilding-ICCV2017.pdf`, if you wish. 
