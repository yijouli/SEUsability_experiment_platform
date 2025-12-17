import argparse

from .installer import ReallmInstaller
import reallm


def main():
    parser = argparse.ArgumentParser(
        description="""
        The Reallm CLI is used to install and run the Reallm plugin.
        """,
        epilog="""
        Examples:
        reallm -i 
        """
    )
    parser.add_argument(
        "-i", "--install", action="store_true", help="Install DAILA into your decompilers"
    )
    parser.add_argument(
        "--single-decompiler-install", nargs=2, metavar=('decompiler', 'path'),
        help="""
        Install REaLLM into a single decompiler. Decompiler must be one of: ida, ghidra, binja, angr.
        """
    )
    parser.add_argument(
        "-v", "--version",
        action="version",
        version=f"{reallm.__version__}"
    )
    args = parser.parse_args()

    if args.single_decompiler_install:
        decompiler, path = args.single_decompiler_install
        ReallmInstaller().install(interactive=False, paths_by_target={decompiler: path})
    elif args.install:
        ReallmInstaller().install()
