o = open("labs.txt", "w")
i = open("input.txt", "r")
names = {}
labs = {}
with open("data.txt") as f:
    for line in f:
       (uo,name,lab) = line.split(';')
       names[uo] = name
       labs[uo] = lab
for x in i:
  if "Session Name" in x:
    user = x.split("-")[1]
  if "IP" in x:
    ip = x.split("|")[1].replace("(DHCP)", "").rstrip()
  if "Location" in x:
    pc_name = x.split("'")[1]
    if not "NAT" in user:
      if not "MASTER" in user:
        o.write("{0}-{1};{2};{3}".format(user,names[user],ip,labs[user]))
        
i.close()
o.close()