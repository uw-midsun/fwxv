import os
import uuid
from dotenv import load_dotenv
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi

# pip install python-dotenv

def init_mongodb():
    load_dotenv()
    username = os.environ.get("MONGODB_USERNAME")
    password = os.environ.get("MONGODB_PASSWORD")

    uri = "mongodb+srv://" + username + ":" + password + "@telemetry.ph2gl5d.mongodb.net/?retryWrites=true&w=majority&appName=Telemetry"

    # Create a new client and connect to the server
    client = MongoClient(uri, server_api=ServerApi('1'))

    # Send a ping to confirm a successful connection
    try:
        client.admin.command('ping')
    except Exception as e:
        print(e)
    
    return client

def upload_mongodb(dict, collection):
    try:
        dict['_id'] = str(uuid.uuid4())
        collection.insert_one(dict)
        print("Insertion Success")
    except Exception as e:
        print(e)
