# uBAD - Universal Boot Authentication Device  
uBAD is a **next-generation authentication solution** designed to streamline access security while eliminating the hassle of password fatigue.  

![IMG_20250402_201810_146](https://github.com/user-attachments/assets/099def0a-8020-4c19-bede-c6c74a7121ba)  

## Why uBAD?  
Traditional **two-factor authentication (2FA) devices** add extra steps to the login process, slowing users down while still requiring them to **remember or retrieve multiple credentials**. Password fatigue—the frustration of managing long, complex, frequently changed passwords—is a **major security risk**, leading users to rely on weak, reused, or written-down passwords.  

**uBAD solves this problem** by acting as a **physical authentication key**, storing **highly secure, randomized** passwords that users **never need to memorize**. It removes unnecessary delays, making login **faster, more secure**, and **far more convenient than traditional 2FA devices**.  

## Features  
- **Automated Boot Authentication**: Instantly boots **VeraCrypt-encrypted Windows systems** without requiring users to manually enter credentials.  
- **Universal Password Entry**: Can be **programmed to input passwords anywhere** a USB keyboard is accepted.  
- **Maximum Security Passwords**: Supports **randomized credentials up to 64 characters**, ensuring **unbreakable security** without the burden of memorization.  
- **Eliminates Weak Password Practices**: Protects organizations where frequent password rotations lead to risky behaviors like **password reuse or note-taking**.  
- **Physical Key Protection**: Credentials are safeguarded by **1024 switch code combinations**, preventing unauthorized access.  

## Fail-Safe Security Mechanism  
uBAD incorporates a **switch code safeguard** to ensure that only authorized users can access stored credentials:  
- The **correct switch code** must be entered **on the first attempt**.  
- If entered incorrectly, a **red warning light** activates and triggers a **five-second countdown**.  
- During this brief window, the user has the chance to **remove the device** before it wipes credentials permanently.  
- If the device **remains inserted beyond five seconds** OR a second incorrect code is entered, all stored credentials are **wiped completely**.  

## Faster, Smarter Authentication  
Unlike cumbersome 2FA devices that require additional steps, **uBAD streamlines the authentication process**, enabling users to **log in instantly** without needing additional verification codes.  

With **intuitive automation** and a **fail-safe security mechanism**, uBAD eliminates **password fatigue**, prevents credential theft, and redefines secure access.  

--------------------------------------------------------------------------------------------------  
<div align="center">
    <strong>How to Setup Device</strong>
</div>

--------------------------------------------------------------------------------------------------  
## Configuration Tool  
**You must set the switch code prior to configuring the device.** The recommended configuration tool for both Windows and Linux is Putty. For Windows, you can download it [here](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html). For Linux, you can install it with 'sudo apt install putty'.  
  - ### Finding the Device  
    - <ins>**Using Windows:**</ins>  
	  - Plug in device
	  - Click on the search icon in the taskbar  
	  - Search for and open 'Device Manager'  
	  - Expand 'Ports' section  
	  - Look for 'COM_number_'. For example, 'COM19'. If you have more than one device listed under ports, unplug the device and look for the new COM number that shows up when you plug it back in.  
	- <ins>**Using Linux:**</ins>  
	  - Typically the port will be '/dev/ttyACM0' but use command 'dmesg | grep tty' after plugging in device and look for the highest ACM number.  
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
    - <ins>**Automatic VeraCrypt-Windows System Decryption:**</ins> This is primarily what uBAD was designed to do. This mode seamlessly automates the boot process for VeraCrypt-encrypted Windows systems. When using this feature, credentials will be entered approximately 15 seconds after computer is powered on or rebooted. This feature is especially helpful during updates to the Windows system, as the computer will reboot several times and you don't need to manually enter your password each time it does. This option is only for Windows systems encrypted with VeraCrypt. For encrypted drives, folders, Luks, etc., you must use Manual Decryption Mode.  
    - <ins>**Manual Decryption (Enters Credentials on Plugin):**</ins> Use this mode for login credentials, Luks encrypted systems, encrypted drives, encrypted folders, etc. Credentials will be entered immediately after plugging the device in. Cursor must be in the password box before plugging in device, as it acts just like a keyboard. This mode is not compatible with PIMs unless they are entered prior to plugging in the device.  
  - ### Permanently Disabling Firmware Updates
    This device has the ability to update it's application firmware by default. This is so that users can verify or modify the way the device functions, or if security vulnerabilities are discovered in the future, they can be patched. Whether choosing to enable or disable this feature, there are risks. If you choose to disable it, the change is permanent. You will never be able to address any future vulnerabilities that may be discovered and will need to replace the device to protect against them. If you leave the update feature enabled, you will be at risk of a 'trojan-horse attack'. While the factory firmware prevents your credentials from being exposed if the switch-code is incorrect, an attacker with physical access to the device could upload malicious code that could bypass all these security features. Your credentials will always be wiped from the device prior to any firmware updates, so the risk here is re-entering credentials into a compromised device that an attacker may be able to access again in the future. If you leave the update feature enabled and find your credentials unexpectedly erased, it is highly recommended that you perform an update with the official source code before re-entering credentials. This will help mitigate any potential trojan-horse attack. Again, disabling firmware updates will prevent this kind of an attack altogether, but you would need to replace the device if any security vulnerabilities are discovered in the future if you want to protect against them.  
  - ### Decryption Password  
    - This is your password for encrypted systems or really anything. The maximum length of this password is 64 characters.  
  - ### VeraCrypt System PIM  
    - This is only for Veracrypt Encrypted Systems. Encrypted drives, folders, etc. that use a PIM must enter it manually and prior to plugging in the device.  

