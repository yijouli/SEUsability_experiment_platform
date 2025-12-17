#!/usr/bin/env python3
import sys
import os
import pycparser
import networkx as nx
import matplotlib.pyplot as plt
from pycparser import c_parser, c_ast, parse_file


class FuncDefVisitor(c_ast.NodeVisitor):
    def __init__(self):
        self.call_graph = nx.DiGraph()
        self.current_function = None

    def visit_FuncDef(self, node):
        self.current_function = node.decl.name
        self.call_graph.add_node(self.current_function)
        self.generic_visit(node)

    def visit_FuncCall(self, node):
        if self.current_function is not None:
            self.call_graph.add_edge(self.current_function, node.name.name)
        self.generic_visit(node)

class ASTMerger(c_ast.NodeVisitor):
    def __init__(self):
        self.merged_ast = c_ast.FileAST([])

    def visit(self, node):
        if isinstance(node, c_ast.FileAST):
            self.merged_ast.ext.extend(node.ext)
        else:
            super().visit(node)

def parse_and_merge_files(file_list):
    merger = ASTMerger()
    for file in file_list:
        ast = parse_file(file, use_cpp=True)
        merger.visit(ast)
    return merger.merged_ast


def plot_call_graph(project_folder: str):
    c_sources = [os.path.abspath(os.path.join(project_folder, f)) for f in os.listdir(project_folder) if f.endswith('.c')]
    merged_ast = parse_and_merge_files(c_sources)
    visitor = FuncDefVisitor()
    visitor.visit(merged_ast)

    pos = nx.spring_layout(visitor.call_graph)
    nx.draw(visitor.call_graph, pos, with_labels=True, node_size=2000, node_color="skyblue", font_size=10, font_weight="bold", arrows=True)
    plt.show()


if __name__ == "__main__":
    # if len(sys.argv) != 2:
    #     print("Usage: python script.py <source_code_file>")
    #     sys.exit(1)
    # folder = sys.argv[1]
    folder = "/home/simo/gitAcademy/chatgpt-for-RE/challenges/secrets"
    plot_call_graph(folder)
