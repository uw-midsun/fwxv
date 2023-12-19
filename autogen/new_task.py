from pathlib import Path


def get_data(args):
    task_name = Path(args.output).stem
    proj_name = Path(args.output).parent.stem

    return {"proj_name": proj_name, "task_name": task_name}
