from flask import Flask, request, jsonify
import binascii
import numpy as np
from PIL import Image
import io
import base64

app = Flask(__name__)

@app.route('/upload', methods=["POST"])
def upload_image():
	png_magic = b'\x89\x50\x4e\x47\x0d\x0a\x1a\x0a'
	try:
		data = request.get_json()
		hex_data = data['imageData']
		byte_string = bytes.fromhex(hex_data)
		byte_string = png_magic + byte_string
		print(byte_string)
#		base64_data = data['imageData']
#		print(type(base64_data))
#		if not base64_data:
#			return jsonify({"error": "No imageData provided"}), 400
#		try:
#			binary_data = base64.b64decode(base64_data)
#			binary_data = png_magic + binary_data
#			print(repr(binary_data))
#		except base64.binascii.Error as e:
#			return jsonify({"error": "Invalid Base64 data: " + str(e)}), 400
#
		#try:
		#	image = Image.open(io.BytesIO(byte_string))
		#	image = image.resize((50,50))
		#	image = image.convert("L")
		#	image.save("output_image.png")
		#except Exception as e:
			#return jsonify({"error": "Error processing image: " + str(e)}), 400
		with open("output_image.png", "wb") as fh:
			fh.write(byte_string)
		return jsonify({"message": "Image uploaded and processed successfully"}), 200
	except Exception as e:
		return jsonify({"error": str(e)}), 400

@app.route("/")
def hello():
	return "Hello"

