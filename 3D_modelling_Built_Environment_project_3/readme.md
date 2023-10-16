# GEO1004 - Readme HW03

Names:

* **Dmitri Visser** (4279913)
* **Katrin Meschin** (5163889)
* **Cynthia Cai** (5625483)

## Preparation

Using the file ``IfcOpenHouse_IFC4.ifc``, preprocess IFC file with *Ifcconvert*, e.g.:

    $ .\IfcConvert.exe --orient-shells --include+=entities IfcRoof IfcWall IfcFooting IfcWindow IfcDoor --weld-vertices --validate .\IfcOpenHouse_IFC4.ifc output.obj


The project driver code is contained by ``main.cpp``.

## Methodology to reproduce output

The project can be compiled as follows:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ ./hw03 ([DIR] | [INPUT_FILE] [OUTPUT_FILE] | [INPUT_FILE] [OUTPUT_FILE] --write-off )


* **Run with two arguments:**

        $ ./hw02 input.obj output.json

  Provide ``INPUT_FILE`` as first argument. Provide ``OUTPUT_FILE`` as second argument.

* **Run with three arguments:**

        $ ./hw02 input.city.json output.city.json --write-off

  Same as with two arguments.
  
  Specifying `--write_off` enables verbose export of all polyhedrons to off files.


* **Run without arguments:**

        $ ./hw02

  If no input file is provided, the input file will be retrieved from ``../../data/input.obj``. If no output file is provided, the output file will be written to ``../../data/output.json``.


The output file will be generated in one run.