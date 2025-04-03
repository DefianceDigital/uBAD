# uBAD - universal Boot Authentication Device  
uBAD is a cutting-edge security solution designed to safeguard sensitive access credentials with unparalleled protection.  
![IMG_20250402_201810_146](https://github.com/user-attachments/assets/099def0a-8020-4c19-bede-c6c74a7121ba)  

## Features
- **Automated Boot Authentication**: Seamlessly boots VeraCrypt-encrypted Windows systems for a streamlined and secure login experience.
- **Versatile Credential Management**: Can be programmed to enter passwords anywhere a USB keyboard is accepted.
- **Enhanced Password Security**: Supports passwords up to **64 characters**, allowing highly secure, randomized credentials that are virtually impossible to memorize.
- **Ideal for Organizations**: Helps mitigate the risks of employees reusing or writing down credentials in accessible locations.

## Physical Key & Switch Code Security
uBAD operates as a **physical key**, equipped with a **switch code safeguard** to ensure access security:
- Users must enter the switch code correctly on the first attempt.
- An incorrect entry triggers a **red warning light** and initiates a **five-second countdown**.
- During this brief window, the user has the opportunity to remove the device.
- If the device remains inserted beyond five seconds, stored credentials will be **wiped completely**.
- Alternatively, if a second incorrect switch code is entered, stored credentials will be **wiped completely**.

## Security & Authentication Redefined
uBAD combines **intelligent automation and an innovative fail-safe mechanism** to protect digital access while eliminating the risks of weak password management.   

# How to use

## Configuration Tool  
The recommended configuration tool for both Windows and Linux is Putty. For Windows, you can download it [here](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html). For Linux, you can install it with 'sudo apt install putty'.  
  - ### Finding the Device  
    - <ins>**Using Windows:**</ins>  
	  - Plug in device
	  - Click on the search icon in the taskbar  
	  - Search for and open 'Device Manager'  
	  - Expand 'Ports' section  
	  - Look for 'COM_number_'. For example, 'COM19'. If you have more than one device listed under ports, unplug the device and look for the new COM number that shows up when you plug it back in.  
	- <ins>**Using Linux:**</ins>  
	  - Typically the port will be '/dev/ttyACM0' but use command 'dmesg | grep tty' to find other ACM devices if more than one are plugged in.  
	  **If nothing prints, unplug the device and plug it back in.**  
  - ### Launching Putty  
    - <ins>**Using Windows:**</ins>  
	  - Search for and open 'PuTTY'  
	- <ins>**Using Linux:**</ins>  
	  - Enter command 'sudo putty'  
  - ### Connecting to Device  
    - Click 'Serial' and enter the port discovered above.  
    ![selport](https://github.com/user-attachments/assets/c3c1f699-0555-4b1d-94c5-d95c979f6ca0)  
    - Click on 'Terminal' in the left menu and set both 'Line discipline options' to 'Force on'.  
    ![discipline](https://github.com/user-attachments/assets/3b6f58ee-b769-4da4-85f5-0f7861a5d3d9)  
    - Click 'Open' to begin device configuration.  
	
	**Note: You can paste data to putty by holding Ctrl key and right clicking in the window.**  

## Configuration Options  
  ![config](https://github.com/user-attachments/assets/a4a8c259-ceff-4ea9-b696-330103f0ebc6)  
  - ### Decryption Modes:
    - <ins>**Automatic VeraCrypt-Windows System Decryption:**</ins> This is primarily what uBAD is designed to do. This option is capable of determining what state the Windows computer is in, and will enter the VeraCrypt password whenever it is on the VeraCrypt boot screen. This feature is especially helpful during updates to the Windows system, as the computer will reboot several times and you don't need to manually enter your password each time it does. This option is only for Windows systems encrypted with VeraCrypt. For encrypted drives, folders, Luks, etc., you must use Manual Decryption Mode.  
    - <ins>**Manual Decryption (Enters Credentials on Plugin):**</ins> Use this mode for Luks encrypted systems or encrypted drives, folders, etc. Credentials will be entered immediately after plugging the device in. Cursor must be in the password box before plugging in device. This mode is not compatible with PIMs unless they are entered prior to plugging in the device.  
  - ### Permanently Disabling Firmware Updates
    This device has the ability to update it's application firmware by default. This is so that users can verify or modify the way the device functions, or if security vulnerabilities are discovered in the future, they can be patched. Whether choosing to enable or disable this feature, there are risks. If you choose to disable it, the change is permanent. You will never be able to address any future vulnerabilities that may be discovered and will need to replace the device to protect against them. If you leave the update feature enabled, you will be at risk of a 'trojan-horse attack'. While the factory firmware prevents your credentials from being exposed if the switch-code is incorrect, an attacker with physical access to the device could upload malicious code that could bypass all these security features. Your credentials will always be wiped from the device prior to any firmware updates, so the risk here is re-entering credentials into a compromised device that an attacker may be able to access again in the future. If you leave the update feature enabled and find your credentials unexpectedly erased, it is highly recommended that you perform an update with the official source code before re-entering credentials. This will help mitigate any potential trojan-horse attack. Again, disabling firmware updates will prevent this kind of an attack altogether, but you would need to replace the device if any security vulnerabilities are discovered in the future if you want to protect against them.  
  - ### Decryption Password  
    - This is your password for Luks, Veracrypt System, Drive, Folder, or really anything. The maximum length of this password is 64 characters.  
  - ### VeraCrypt System PIM  
    - This is only for Veracrypt Encrypted Systems. Encrypted drives, folders, etc. that use a PIM must enter it manually and prior to plugging in the device.  

