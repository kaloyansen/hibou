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

```bash
sudo zypper install ./hibou.rpm
```


## **Install from source**

```bash
git clone git@github.com:kaloyansen/hibou
cd hibou
mkdir bin
gcc src/hibou.c -o bin/hibou -lncurses
bin/hibou
```
