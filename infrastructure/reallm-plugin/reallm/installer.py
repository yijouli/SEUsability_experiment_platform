import textwrap
from pathlib import Path

from libbs.plugin_installer import LibBSPluginInstaller


class ReallmInstaller(LibBSPluginInstaller):
    def __init__(self):
        super().__init__()
        self.pkg_path = self.find_pkg_files("reallm")

    def _copy_plugin_to_path(self, path):
        src = self.pkg_path / "reallm_plugin.py"
        dst = Path(path) / "reallm_plugin.py"
        self.link_or_copy(src, dst, symlink=True)

    def display_prologue(self):
        print(textwrap.dedent("""
        Now installing...
        
        ██████  ███████  █████  ██      ██      ███    ███ 
        ██   ██ ██      ██   ██ ██      ██      ████  ████ 
        ██████  █████   ███████ ██      ██      ██ ████ ██ 
        ██   ██ ██      ██   ██ ██      ██      ██  ██  ██ 
        ██   ██ ███████ ██   ██ ███████ ███████ ██      ██ 
        
        The human-study LLM plugin.
        """))

    def install_ida(self, path=None, interactive=True):
        path = super().install_ida(path=path, interactive=interactive)
        if not path:
            return

        self._copy_plugin_to_path(path)
        return path

    def install_ghidra(self, path=None, interactive=True):
        path = super().install_ghidra(path=path, interactive=interactive)
        if not path:
            return

        self._copy_plugin_to_path(path)
        return path

    def install_binja(self, path=None, interactive=True):
        path = super().install_binja(path=path, interactive=interactive)
        if not path:
            return

        self._copy_plugin_to_path(path)
        return path

    def install_angr(self, path=None, interactive=True):
        path = super().install_angr(path=path, interactive=interactive)
        if not path:
            return

        path = path / "Reallm"
        path.mkdir(parents=True, exist_ok=True)
        self._copy_plugin_to_path(path)
        return path