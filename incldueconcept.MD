Syntax :

Include "location"

Where location is the absolute or relative location of another source file from the present path.
The inclusion will happen in a preprocessing phase, before scanner even starts. Includer will append all of the included sources content to the present buffer, and will repeat the process over and over again until all Include statement is completed. The includer will also maintain a global inclusion table to prevent from duplicate inclusions. Since all sources are interpreted at runtime, if two or more files includes method with same names, bad things will happen. Hence it is advised to write as unique function identifers in libraries as possible. Alang is targeted towards simplicity, hence it'll not provide any way of controlling access or scopes of variables and functions.
