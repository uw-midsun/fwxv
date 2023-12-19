import argparse
import jinja2
from pathlib import Path


def get_file_name(template_name, board):
    # get the name of the jinja file from the filepath
    jinja_prefix = Path(template_name).stem
    # files that start with _ are generic and we want to prepend the board name
    if jinja_prefix.startswith('_') and board != None:
        return board + jinja_prefix
    else:
        return jinja_prefix


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("template_name")
    parser.add_argument("-o", dest="output", required=True, metavar="DIR")

    args = parser.parse_args()
    print(args.template_name)

    if args.template_name in ["can", "can_board_ids", "python_can"]:
        from .can import get_data
    elif args.template_name in ["test"]:
        from .test import get_data
    elif args.template_name in ["new", "new_py"]:
        from .new import get_data
    elif args.template_name in ["new_task"]:
        from .new_task import get_data

    data = get_data(args)

    template_dir = Path("autogen/templates", args.template_name)
    template_loader = jinja2.FileSystemLoader(searchpath=template_dir)
    env = jinja2.Environment(loader=template_loader)
    env.tests["contains"] = (lambda list, var: (var in list))

    for template in template_dir.glob("**/*.jinja"):
        template_path = str(template.relative_to(template_dir))

        output = env.get_template(template_path).render(**data)
        output_file_name = env.from_string(template_path).render(**data)
        output_file = Path(args.output, output_file_name).with_suffix("")
        output_file.parent.mkdir(parents=True, exist_ok=True)
        output_file.write_text(output)

    print("Done autogenerating")


if __name__ == "__main__":
    main()
