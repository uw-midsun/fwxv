from pathlib import Path


def get_data(args):
    project_name = Path(args.output).stem
    project_type = Path(args.output).parent.stem

    libs = []
    if project_type in ["projects", "smoke"]:
        # default libs
        libs = ["FreeRTOS", "ms-common"]
    
    if project_type == "py":
        args.template_name = "new_py"
    
    if Path(args.output).exists():
        raise (f"{project_type}/{project_name} already exists")

    return {"proj_name": project_name, "libs": libs }

