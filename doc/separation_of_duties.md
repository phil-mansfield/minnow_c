# What minnow Does and Does Not Do

minnow is a library, not a toolchain. This means that there is a very
clearly defined boundary between minnow's responsibilities and your
responsibilities as the person using it.

| Client Responsibilities | minnow Responsibilities |
|-------------------------|-------------------------|
| multi-node parallelization | single-node multi-threading |
| opening and closing files | reading and writing to open files |
| determining target accuracy | maintaining requested accuracy |
| optimally ordering data | maintaining orginal data order |

The reason for this divide is that everything on the left side of the
table is highly dependent on simulation and system specifics and would
force excessive configuration (as an example of this, see how the Shellfish
configuration files handle directory structure).
