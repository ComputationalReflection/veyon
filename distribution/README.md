# Linux and Windows scripts

These are the Windows and Linux scripts implemented to install, configure and run the remote lab infrastructure described in the article _An Infrastructure to deliver Synchronous Remote Programming Labs_, by [Miguel Garcia](http://www.reflection.uniovi.es/miguel), [Jose Quiroga](http://www.reflection.uniovi.es/quiroga) and [Francisco Ortin](http://www.reflection.uniovi.es/ortin).


## Students

- (`un`)`install`. The `install` script installs the Veyon service, opens the 11100 port in the firewall, includes the public certificate of the lecturer delivering the lab, and sets the authentication method as public-key file authentication.
In order to access the student’s computer, the accessing lecturer must first authenticate himself. Access without authentication is not supported in Veyon. The simplest approach is to include the lecturer’s public key file in the `install` script, and set the authentication mode of Veyon service to key-file authentication. In this way, only the lab lecturer assigned to each student could monitor their work.
The `uninstall` script performs the reverse process.

- (`re`)`start`. One script runs the Veyon service as a background process, and the other one restarts it.

- `stop`. This script stops the Veyon service running in the student’s computer. In this way, students control exactly when they could be monitored. In the `install` script, we set the `autostart` property to false, meaning that the student must run the Veyon service explicitly (i.e., with `start`).

## Lecturers

In the case of lecturers, the scripts to be run need information about students, which are taken from the University Enterprise Resource Planning (ERP). That information is stored in a database that includes student name and surname, their unique university ID, the lab they belong to, and the ID of the assigned lecturer.

These are the four scripts we provide for lecturers:

- (`un`)`install`. Very similar to student scripts. The main difference is that this script installs the lecturer’s public and private keys, so he/she must provide these two files before installation.

- `create_labs`. This script connects to the VPN server to know all the computer IPs connected to the VPN. For each IP, it searches in the student database for the University ID used by each student connected to the VPN. Then, it calls the `parse` script (below) to create the remote lab configuration file that is passed to the Veyon master application. The outcome is that the Veyon master shows the lecturer all the students’ screens and identification data (name, surname and ID) attending the remote lab. 

- `parse`. This script parses all the information returned by the VPN server. For each IP, it retrieves the associated student ID and takes from the student database their name, surname and the lab they belong to. Finally, it writes all that information in a CSV file used to configure Veyon master through its Common Line Interface (CLI).

# License

Copyright (c) 2020 [Miguel Garcia](http://www.reflection.uniovi.es/miguel) and [Jose Quiroga](http://www.reflection.uniovi.es/quiroga) / [University of Oviedo](http://www.uniovi.es).

See the file COPYING for the GNU GENERAL PUBLIC LICENSE.
