This directory may be used for Visual Studio project, which is made using the `Make-vs-project.r3` script.

##Note
You should not add source files directly in the Visual Studio as it would go out of sync with the other building methods and all  Visual Studio files are excluded from the repository.

Running the `Make-vs-project.r3` script multiple times will overwrite existing project and create a new project id! Maybe there could be another script, which would update just the source files if needed.