## 🐋 Setting Up a Veyon Test Environment with Docker

This guide helps you set up an educational monitoring system with a teacher and multiple student machines using Docker containers.

---

### 📦 Prerequisites

**Docker** installed on your system

---

### ⚙️ Included Configuration

- **Lecturer:**
    - Veyon preconfigured in master mode
    - 2 predefined students
    - Integrated VNC server
- **Students:**
    - Veyon in client mode
    - RDP server enabled
    - Full graphical access

---

### 🔨 Building Docker Images

**Lecturer Image:**

```bash
docker build --target lecturer -t veyon_assistant:lecturer .
```

**Student Image:**

```bash
docker build --target student -t veyon_assistant:student .
```


---

### 🌐 Network Setup

Create a virtual network for the containers:

```bash
docker network create --driver bridge --subnet 172.20.0.0/24 veyon_vnet
```


---

### 👨‍🏫 Running the Lecturer Container

```bash
docker run --rm --name lecturer \
  -e VNC_USER=ubuntu -e VNC_PW=ubuntu \
  -e VNC_RESOLUTION=1920x1080 \
  -e XDG_RUNTIME_DIR=/tmp/runtime-root \
  -p 5901:5901 -p 6901:6901 \
  --net veyon_vnet --ip 172.20.0.20 \
  -v ./screenshots:/home/ubuntu/veyon-screenshoots \
  veyon_assistant:lecturer
```

**Key parameters:**

- `-p 6901:6901`: Web VNC access
- `--ip 172.20.0.20`: Fixed IP on the virtual network
- `-v ./screenshots`: Volume for saving screenshots

---

### 👨‍🎓 Running Student Containers

**Student 1:**

```bash
docker run --rm --name student01 \
  --privileged --cgroupns=host \
  --tmpfs=/run --tmpfs=/tmp \
  -v /sys/fs/cgroup:/sys/fs/cgroup:rw \
  -p 3391:3389 \
  --net veyon_vnet --ip 172.20.0.21 \
  veyon_assistant:student
```

**Student 2:**

```bash
docker run --rm --name student02 \
  --privileged --cgroupns=host \
  --tmpfs=/run --tmpfs=/tmp \
  -v /sys/fs/cgroup:/sys/fs/cgroup:rw \
  -p 3392:3389 \
  --net veyon_vnet --ip 172.20.0.22 \
  veyon_assistant:student
```

**To add more students:**

- Increment the number in `--name` (e.g., `student03`)
- Use a new IP (e.g., `172.20.0.23`)
- Assign a new RDP port (e.g., `-p 3393:3389`)

---

### 🔍 Using the System

**Access the Students:**

1. Use an RDP client
2. Connect to:
    - **Student 1:** `<DOCKER_HOST_IP>:3391`
    - **Student 2:** `<DOCKER_HOST_IP>:3392`
3. Username: `ubuntu`
4. Password: `ubuntu`

**Access the Lecturer:**

1. Open your browser to: `http://<DOCKER_HOST_IP>:6901/vnc.html`
2. Username: `ubuntu`
3. Password: `ubuntu`
4. Go to: `Education → Veyon Master`


---

### 🧹 Cleanup

To stop all containers and remove the network:

```bash
docker stop lecturer student01 student02
docker network rm veyon_vnet
```

> 💡 **Note:** Containers are automatically removed when stopped thanks to the `--rm` flag. Images remain stored locally for future use.
