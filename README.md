# uBAD
Universal Boot Authentication Device for VeraCrypt and Luks

## Device Configuration

### Encryption Modes:
  - <u><b>Automatic VeraCrypt System Decryption (Windows):</b></u> test  
  - **Automatic Luks System Decryption (Linux):** test  
  - **For Manual Decryption (Widows/Linux):** test  
### Permanently Disabling Firmware Patches
This device has the ability to update it's application firmware by default. This is so that users can verify or modify the way the device functions, or if security vulnerabilities are discovered in the future, they can be patched. Whether choosing to enable or disable this feature, there are risks. If you choose to disable it, the change is permanent. You will never be able to address any future vulnerabilities that may be discovered and will need to replace the device to protect against them. If you leave the update feature enabled, you will be at risk of a 'trojan-horse attack'. While the factory firmware prevents your credentials from being exposed if the switch-code is incorrect, an attacker with physical access to the device could upload malicious code that could bypass all these security features. Your credentials will always be wiped from the device prior to any firmware updates, so the risk here is re-entering credentials into a compromised device that an attacker may be able to access again in the future. If you leave the update feature enabled and find your credentials unexpectedly erased, it is highly recommended that you perform an update with the official source code before re-entering credentials. This will help mitigate any potential trojan-horse attack. Again, disabling firmware updates will prevent this kind of an attack altogether, but you would need to replace the device if any security vulnerabilities are discovered in the future if you want to protect against them.
