How to run HTTP apps on GCE
====

These are the steps to create a Mojo app that handles some URLs and push it to run on Google Compute Engine.


## Set up Google Cloud Platform
Install Google Cloud Platform SDK: https://cloud.google.com/sdk/


## Authenticate
You'll need to set a project for the Google Cloud Platform scripts. If you're a Googler, note that you shouldn't use a corporate card for this.

If you're on the Mojo team, ping jam to get added to the "mojodemos" project first.

If you're not on Mojo team, then create a project on Google Compute Engine and replace "mojodemos" with your project-id below.  Also make sure to enable the "Google Compute Engine" API" at your project's console https://console.developers.google.com


```
gcloud auth login
gcloud config set project mojodemos
```


## Create a VM
Now create a VM that'll run the binary. The Debian image doesn't work because of some GLib dependencies in our binaries that we should remove (http://crbug.com/433886). Until then, you have to use the Ubuntu image:
```
gcloud compute instances create YOUR-VM-NAME-HERE --image ubuntu-1404-trusty-v20141031a  --image-project ubuntu-os-cloud --zone us-central1-a
```

Make note of the external IP address of this machine; you'll use this later to connect to the Mojo app.

Install some required packages:
```
gcloud compute ssh YOUR-VM-NAME-HERE --command "sudo apt-get install libgconf-2-4 -y" --zone us-central1-a
gcloud compute ssh YOUR-VM-NAME-HERE --command "sudo apt-get install libnss3 -y" --zone us-central1-a
```

If you're not using mojodemos project-id and created a new project, add a firewall rule to allow port 80:
```
gcloud compute firewall-rules create allow-http  --description "Incoming http allowed." --allow tcp:80
```


## Push the Mojo binaries
Create a directory to hold the binaries:
```
gcloud compute ssh YOUR-VM-NAME-HERE --command "mkdir ~/mojo" --zone us-central1-a
```

For this example, we'll use the examples/http_handler binary which is a minimal Mojo HTTP app. Assuming you've built the release binary:

```
gcloud compute copy-files out/Release/libhttp_handler.so out/Release/libhttp_server.so out/Release/libnetwork_service.so out/Release/mojo_shell YOUR-VM-NAME-HERE:~/mojo --zone us-central1-a
```


## Run the Mojo app
```
gcloud compute ssh YOUR-VM-NAME-HERE --command "sudo mojo/mojo_shell mojo://http_handler" --zone us-central1-a
```

Now you can visit the IP address from the previous step.


## Delete the VM
Once you're not using the VM anymore:
```
gcloud compute instances delete YOUR-VM-NAME-HERE --zone us-central1-a
```
