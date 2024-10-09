import sys
from flash_application import Flash_Application
from jump_application import Jump_Application
from jump_bootloader import Jump_Bootloader

def get_node_ids():
    while True:
        node_ids_input = input("Enter node IDs (comma-separated, e.g., 1,2,3): ").strip()
        try:
            node_ids = [int(node_id.strip()) for node_id in node_ids_input.split(',') if node_id.strip()]
            if not node_ids:
                raise ValueError()
            return node_ids
        except ValueError:
            print("Invalid input. Please enter valid comma-separated node IDs.")

def main():
    flash_app = None
    jump_app = Jump_Application()
    jump_bootloader = Jump_Bootloader()

    print("Bootloader CLI")
    print("Commands: 'flash', 'app', 'bootloader', 'exit'")

    while True:
        command = input("\nEnter command: ").strip().lower()

        if command == 'exit':
            print("Exiting CLI.")
            break

        elif command == 'flash':
            binary_path = input("Enter the path to the binary file: ").strip()
            node_ids = get_node_ids()

            try:
                flash_app = Flash_Application(binary_path)
                flash_app.start_flash(node_ids=node_ids)
                print(f"Firmware flashed successfully on nodes {node_ids}")
            except Exception as e:
                print(f"Error during flashing: {str(e)}")

        elif command == 'app':
            node_ids = get_node_ids()

            try:
                jump_app.jump_application(node_ids=node_ids)
            except Exception as e:
                print(f"Error during jump: {str(e)}")
        
        elif command == 'bootloader':
            node_ids = get_node_ids()

            try:
                jump_bootloader.jump_bootloader(node_ids=node_ids)
            except Exception as e:
                print(f"Error during jump: {str(e)}")

        else:
            print("Unknown command. Please use 'flash', 'app', 'bootloader', or 'exit'.")

if __name__ == "__main__":
    main()