# Comunication Protocols HTTP Client (REAST API implementation - interaction)

## Explicarea pe scurt a codului:
Am implementat un client ce se foloseste de REST API ul expus de server, pentru a interactiona cu clientul (CLI) si a trimite cereri (primite de la tastatura) serverului, pe care sa le interpreteze ulterior ca SUCCESS sau ERROR, si fiecare caz in parte de eroare in functie de ce returneaza serverul.

## Fisiere si functii importate:
Am adaugat `parson.h` si `parson.c` pentru a lucra mai usor cu formatul JSON. De asemenea am inportat functiile implementate in laboratorul 9 de `compute_post_request` si `compute_get_request` ce se afla in fisierul `requests`. De asemenea `helpers`, pentru a putea "impacheta" mai usor un pachet, de asemenea `buffer.c` si `buffer.h` deoarece sunt utilizate in interiorul `helpers`. De asemenea `utils.h` in care am pus DIE ul.


## Requirements:
	This repository contains checker for the PCom HTTP client homework.

	## Prerequisites

	Dependencies:

	- Python >= 3.7;
	- [`pexpect`](https://pexpect.readthedocs.io/en/stable/) (third party package for console automation);
	- [`pyyaml`](https://pypi.org/project/PyYAML/) (third party package for YAML);

	It is highly recommended to use a VirtualEnv, either by using the bundled
	Makefile or by manually installing the dependencies:
	```sh
	# symply run:
	make
	# this will do the same as:
	python3 -mvenv .venv
	.venv/bin/python3 -mpip install -r requirements.txt
	# Note: you need to source activate each time you start a new terminal
	source .venv/bin/activate
	```

	### Usage

	Invoke the checker on your client's compiled executable:

	```sh
	# !!! don't forget to source .venv/bin/activate !!!
	# first, read the tool's internal help:
	python3 checker.py --help 
	# run the checker using default settings:
	python3 checker.py ../path/to/client
	# you MUST supply a valid admin user & password!
	python3 checker.py --admin 'myadminuser:hunter2' ../path-to/client
	```

	The default test script uses the admin user to create a random normal test user.
	This will ensure a clean slate while doing all other tests (since the server 
	persists all edits inside a database).

	Alternately, you can use e.g., `--script CLEANUP` if you have a functioning
	implementation for `get_users` and `delete_user` to quickly cleanup your
	associated users & other database items.

	Also make sure to check out [the source code](./checker.py) for the
	actual details about the script(s) being tested.

	<span style="color: #A33">**Warning**: This _alpha version!_ script is just an 
	instrument used by our team to automate the homework verification process.
	If any bugs affecting its effectiveness are found, we reserve the right to
	correct them at any time (you will be notified when this is the case).
	When in doubt, use the homework text as the rightful source of truth and use the
	Moodle Forum to ask any questions.
	</span>


## Descrierea pe larg a fiecarei functii:

## login_admin_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem deja conectati  
- Citim credentialele de autentificare  
- Deschidem conexiunea cu serverul  
- Pregatim datele pentru autentificare  
- Cream cererea POST pentru autentificare  
- Trimitem cererea la server  
- Primim raspunsul  
- Verificam daca autentificarea a avut succes  
- Extragem cookie-ul de sesiune din raspuns  
- Eliberam memoria  

## add_user_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati ca admin  
- Citim datele utilizatorului nou  
- Deschidem conexiunea  
- Pregatim datele utilizatorului  
- Adaugam cookie-ul de sesiune  
- Cream cererea POST pentru adaugare utilizator  
- Trimitem cererea la server  
- Primim raspunsul  
- Verificam daca adaugarea a avut succes  
- Eliberam memoria  

## get_users_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati ca admin  
- Deschidem conexiunea  
- Adaugam cookie-ul de sesiune  
- Cream cererea GET pentru lista utilizatorilor  
- Trimitem cererea la server  
- Primim raspunsul  
- Verificam si procesam raspunsul  
- Parsam json-ul cu lista de utilizatori  
- Afisam fiecare utilizator din lista  
- Eliberam memoria  

## logout_admin_cmd
- Verificam daca suntem conectati ca admin  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea GET pentru delogare  
- Trimitem cererea la server  
- Primim raspunsul  
- Verificam daca delogarea a avut succes  
- Stergem cookie-ul de sesiune  
- Eliberam memoria  

## login_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem deja conectati ca utilizator  
- Citim datele de autentificare  
- Deschidem conexiunea  
- Pregatim datele de autentificare  
- Folosim cookie-ul de sesiune admin daca exista  
- Cream cererea POST pentru autentificare utilizator  
- Trimitem cererea la server  
- Primim raspunsul  
- Verificam daca autentificarea a avut succes  
- Extragem cookie-ul de sesiune pentru utilizator  
- Eliberam memoria  

## logout_cmd
- Verificam daca suntem conectati ca utilizator  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea GET pentru delogare  
- Trimitem cererea la server  
- Primim raspunsul  
- Verificam daca delogarea a avut succes  
- Stergem cookie-ul de sesiune pentru utilizator  
- Eliberam memoria  

## get_access_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati ca utilizator  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea GET pentru obtinere token  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Extragem token-ul JWT  
- Eliberam memoria  

## get_movies_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea GET pentru obtinerea filmelor  
- Adaugam token-ul JWT in header-ul de autorizare  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Parsam si afisam filmele  
- Eliberam memoria  

## add_movie_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Citim detaliile filmului  
- Deschidem conexiunea  
- Pregatim datele filmului  
- Pregatim cookie-ul de sesiune  
- Cream cererea POST pentru adaugare film  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Eliberam memoria  

## get_movie_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Citim id-ul filmului  
- Deschidem conexiunea  
- Construim URL-ul pentru filmul specific  
- Pregatim cookie-ul de sesiune  
- Cream cererea GET pentru obtinerea detaliilor filmului  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Parsam si afisam detaliile filmului  
- Eliberam memoria  

## update_movie_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Citim id-ul filmului si noile detalii  
- Construim URL-ul pentru filmul care va fi actualizat  
- Deschidem conexiunea  
- Pregatim datele actualizate ale filmului  
- Pregatim cookie-ul de sesiune  
- Cream cererea PUT pentru actualizarea filmului  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Eliberam memoria  

## delete_movie_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Citim id-ul filmului de sters  
- Construim URL-ul pentru filmul care va fi sters  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea DELETE pentru stergerea filmului  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Eliberam memoria  

## add_movie_to_collection_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Construim URL-ul pentru adaugarea filmului in colectie  
- Deschidem conexiunea  
- Pregatim datele cu id-ul filmului  
- Pregatim cookie-ul de sesiune  
- Cream cererea POST pentru adaugarea filmului in colectie  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Eliberam memoria  

## add_collection_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Citim titlul colectiei  
- Citim numarul de filme  
- Deschidem conexiunea  
- Pregatim datele colectiei  
- Pregatim cookie-ul de sesiune  
- Cream cererea POST pentru crearea colectiei  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Extragem id-ul colectiei create  
- Adaugam filmele in colectie  
- Procesam raspunsul pentru crearea colectiei  
- Eliberam memoria  

## get_collections_cmd
- Curat buffer-ul de intrare  
- Verificam daca suntem conectati si avem acces la librarie  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea GET pentru obtinerea colectiilor  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Parsam si afisam colectiile  
- Eliberam memoria  

## get_collection_cmd
- Curat buffer-ul de intrare  
- Citim id-ul colectiei  
- Verificam daca suntem conectati si avem acces la librarie  
- Deschidem conexiunea  
- Construim URL-ul pentru colectia specifica  
- Pregatim cookie-ul de sesiune  
- Cream cererea GET pentru obtinerea detaliilor colectiei  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Primim raspunsul  
- Procesam raspunsul  
- Eliberam memoria  

## delete_collection_cmd
- Curat buffer-ul de intrare  
- Citim id-ul colectiei de sters  
- Construim URL-ul pentru colectia care va fi stearsa  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea DELETE pentru stergerea colectiei  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Eliberam memoria  

## delete_movie_from_collection_cmd
- Curat buffer-ul de intrare  
- Citim id-urile colectiei si filmului de sters  
- Construim URL-ul pentru stergerea filmului din colectie  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea DELETE pentru stergerea filmului din colectie  
- Adaugam header-ul de autorizare cu token JWT  
- Inseram header-ul de autorizare in cerere  
- Trimitem cererea la server  
- Eliberam memoria  

## delete_user_cmd
- Citim username-ul utilizatorului de sters  
- Construim URL-ul pentru stergerea utilizatorului  
- Deschidem conexiunea  
- Pregatim cookie-ul de sesiune  
- Cream cererea DELETE pentru stergerea utilizatorului  
- Trimitem cererea la server  
- Primim raspunsul  
- Procesam raspunsul  
- Eliberam memoria  
