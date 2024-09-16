import json

class Node:
    def __init__(self, name, xpath, regex, children=None):
        self.name = name
        self.xpath = xpath
        self.regex = regex
        self.children = children or []

    def add_child(self, child_node):
        self.children.append(child_node)

    def get_child(self, name):
        """Retrieve a child node by its name."""
        for child in self.children:
            if child.name == name:
                return child
        return None

    def __getattr__(self, name):
        """Allows access to children nodes using dot notation."""
        # Replace dashes with underscores to follow Python naming conventions
        child_node = self.get_child(name)
        if child_node:
            return child_node
        raise AttributeError(f"'{self.__class__.__name__}' object has no attribute '{name}'")

    def __repr__(self):
        return f"Node(name='{self.name}', xpath='{self.xpath}', children={len(self.children)})"

    @classmethod
    def from_dict(cls, name, data):
        """Recursively builds a Node structure from a dictionary."""
        node = cls(name, data.get('xpath', ''), data.get('regex', ''))
        children = data.get('children', {})
        
        # Recursively add children
        for child_name, child_data in children.items():
            child_node = cls.from_dict(child_name, child_data)
            node.add_child(child_node)
        
        return node

def load_tree_from_json(filepath):
    """Loads the tree structure from a JSON file and returns the root node."""
    with open(filepath, "r") as f:
        data = json.load(f)
    
    # Assuming there's a single root node named "root"
    root_name, root_data = next(iter(data.items()))
    
    return Node.from_dict(root_name, root_data)

# Example usage
root_node = load_tree_from_json("/home/luca/Documents/Projects/Head_Hunter_9000/xpaths.json")

# # Access login-main and login-username-input using dot notation
# login_main_node = root_node.login_main
# print(f"Login Main Node: {login_main_node}")

# login_username_input = root_node.login_main.login_username_input
# print(f"Login Username Input XPath: {login_username_input.xpath}")
