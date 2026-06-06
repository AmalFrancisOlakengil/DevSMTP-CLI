## Developer Context Mailer (DevSMTP CLI)

DevSMTP CLI is a highly efficient, terminal-based developer tool designed to bridge the gap between noticing a code issue and notifying the responsible developer. Instead of context-switching to a heavy browser window or an email client, you can flag a line boundary issue, attach the context, and send a structured, encrypted email directly from your production or development terminal.

## Features

Instant Terminal Workflow: Send mails without switching from your active command-line interface.

Code Extraction: Automatically reads a target source file, grabs the exact line range you specify, and embeds it as a formatted snippet.

Secure Encrypted Transmission: Utilizes OpenSSL to establish an authenticated SSL/TLS handshake directly with modern SMTP servers (e.g., Gmail on Port 465).

Silent Credentials Management: Local setup profile saves configurations securely on the machine (~/.config/devsmtp/config.txt), eliminating repetitive login prompts.

## Distribution Directory Structure

Your portable, compiled release folder should look precisely like this:

```
devsmtp-cli/
├── send_mail.exe          # Compiled C++ executable
├── libcrypto-3-x64.dll     # OpenSSL Cryptographic Engine binary
└── libssl-3-x64.dll         # OpenSSL Secure Connection protocol binary
```


## Installation & Setup

Follow these steps to configure the command globally on your machine so it can be run from any workspace directory.

Step 1: Download & Extract

Download the release bundle as a .zip file from the repository releases.

Extract the contents of the ZIP folder into a permanent location on your local drive (e.g., C:\tools\devsmtp or C:\developer\bin).

Note: Do not delete or move this folder after installation, as Windows needs to access the executable and DLLs here.

Step 2: Add to Environment Variables (Path)

To use the send_mail command from any project repository, your operating system needs to know where the executable lives.

# Windows Manual GUI Setup

Open the Windows Start Menu, type "env", and select Edit the system environment variables.

In the System Properties window, click the Environment Variables... button at the bottom right.

Look at the top section labeled User variables for :

Scroll down, select the variable named Path (or PATH), and click Edit....

(If a Path variable does not exist, click New..., set the Variable name to Path, and proceed).

In the Edit window, click the New button on the right side.

Paste the absolute path to your extracted folder (e.g., C:\tools\devsmtp).

Click OK on all three open windows to save your changes and exit.

Restart all active terminal windows or your code editor to load the updated configuration.


## First-Time Setup (Credentials)

The very first time you invoke the tool, it will execute an interactive configuration walkthrough. This saves your sender profile locally, so you only have to do this once.

IMPORTANT SECURITY NOTE: > Modern email providers (like Gmail) block basic account passwords for SMTP. You must use an App Password (a 16-character code generated in your email account's security configuration under 2-Factor Authentication).

## How to Get a Gmail App Password

To generate the required 16-character App Password for a Google/Gmail account:

Navigate to your Google Account Security Panel.

Under the "How you sign in to Google" section, make sure 2-Step Verification is enabled.

Click on 2-Step Verification, scroll to the very bottom of the page, and select App passwords.

Enter a custom name for the app (e.g., DevSMTP CLI) and click Create.

Copy the 16-character password displayed inside the pop-up box (you do not need to copy the spaces). This is the key you will provide to the CLI tool.

##Run the base command without any parameters to trigger the configuration window:

send_mail


## Command Usage

Run the tool from any project folder by passing the source file name, recipient's email, and the specific line boundaries you wish to target:

send_mail <filename> <recipient_email> <start_line> <end_line>


## Example:

send_mail app.py teammate@company.com 14 18


## Interactive Execution Flow:

=== Developer Context Mailer ===
Subject: Buffer optimization trace fault
Issue Description (Body): Mind double-checking the sizing array bound configuration over here? It feels like it could throw a memory layout fault when parsing dynamic blocks.

Issue flagged! Contextual email sent successfully to teammate@company.com.


## What the Recipient Receives

Your teammate will receive a beautifully formatted email displaying your written notes along with the targeted raw code block highlighted automatically:

Subject: [Code Issue] Buffer optimization trace fault

Mind double-checking the sizing array bound configuration over here? It feels like it could throw a memory layout fault when parsing dynamic blocks.

```
----- CODE SNIPPET (app.py) -----
14: def process_payload(buffer, size):
15:     # Bug: missing structural bounds mapping check here
16:     internal_buffer = bytearray(256)
17:     memoryview(internal_buffer)[:size] = buffer[:size]
18: 
-----------------------------------------
```

