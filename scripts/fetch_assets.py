import os
import urllib.request
import ssl

def download_file(url, destination):
    print(f"Downloading {url} to {destination}...")
    try:
        ctx = ssl.create_default_context()
        ctx.check_hostname = False
        ctx.verify_mode = ssl.CERT_NONE
        
        with urllib.request.urlopen(url, context=ctx) as response, open(destination, 'wb') as out_file:
            data = response.read()
            out_file.write(data)
        print("Success!")
    except Exception as e:
        print(f"Error downloading {url}: {e}")

def main():
    assets_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'Assets')
    if not os.path.exists(assets_dir):
        os.makedirs(assets_dir)
        print(f"Created directory: {assets_dir}")

    # Use the official variable font file from the google/fonts repo
    url = "https://raw.githubusercontent.com/google/fonts/main/ofl/orbitron/Orbitron%5Bwght%5D.ttf"
    destination = os.path.join(assets_dir, "Orbitron.ttf")
    
    if not os.path.exists(destination):
        download_file(url, destination)
    else:
        print(f"Orbitron.ttf already exists in {assets_dir}.")

if __name__ == "__main__":
    main()
