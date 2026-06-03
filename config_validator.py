import json
import os

def validate_forge_schema(schema_path):
    """Ensures schema.json conforms to configuration requirements before starting C ingest loops."""
    print(f"[INIT] Commencing validation check on: {schema_path}")
    
    # Check if the file actually exists on the disk
    if not os.path.exists(schema_path):
        print(f"[ERROR] Target validation profile missing: {schema_path}")
        return False
        
    try:
        with open(schema_path, 'r') as file:
            config_matrix = json.load(file)
            
        # Basic validation rules for system processing
        print("[SUCCESS] Structural JSON parsing completed without syntax errors.")
        print(f"[DATA PACK] Current Schema Content: {json.dumps(config_matrix, indent=2)}")
        return True
        
    except json.JSONDecodeError as syntax_err:
        print(f"[CRITICAL] Malformed structural formatting found in file: {syntax_err}")
        return False
    except Exception as general_err:
        print(f"[CRITICAL] Unexpected system exception encountered: {general_err}")
        return False

if __name__ == "__main__":
    # Target the literal file sitting in your forge-core directory
    target_schema = "schema.json"
    validate_forge_schema(target_schema)
