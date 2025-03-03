# **hibou**

## **a compact system resources monitor**

hibou is written in c with ncurses

## **Download RPM**

You can download the latest RPM from:

- **GitHub Actions Artifacts**

  1. Go to [Actions](https://github.com/kaloyansen/hibou/actions)
  2. Select the latest workflow run
  3. Download the **rpm-package** artifact

## **Installation**

Run the following command to install the package:

  openSuse
```bash
sudo zypper in ./hibou.rpm
```

  Fedora
```bash
sudo dnf install ./hibou.rpm
```

  CentOS
```bash
sudo yum install ./hibou.rpm
```

  Debian
```bash
sudo apt install alien
sudo alien -d hibou.rpm
sudo dpkg -i hibou.deb
```


## **Install from source (recommended)**

```bash
git clone git@github.com:kaloyansen/hibou
cd hibou
make
bin/hibou
```
