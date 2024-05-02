from PIL import Image, ImageDraw, ImageFont

def text_to_image(text_path, output_image_path, font_size=12, text_color=(0, 0, 0), background_color=(255, 255, 255), image_width=800, image_height=600):
    # Read the text file
    with open(text_path, "r") as file:
        content = file.read()
        print(content)
    # Create a blank image
    image = Image.new("RGB", (image_width, image_height), background_color)
    draw = ImageDraw.Draw(image)

    # Load a fixed-size font
    font = ImageFont.load_default()

    # Calculate text size (approximate)
    text_width = len(content) * font_size // 2  # Adjust this value based on your font and content
    text_height = font_size

    # Calculate text position
    text_x = (image_width - text_width) // 2
    text_y = (image_height - text_height) // 2

    # Draw text on the image
    draw.text((text_x, text_y), content, fill=text_color, font=font)

    # Save the image
    image.save(output_image_path)

if __name__ == "__main__":
    text_path = "TestCases/processes.txt"  # Path to your text file
    output_image_path = "output_image.png"  # Path to save the output image
    font_size = 12  # Font size
    text_color = (0, 0, 0)  # Text color (RGB)
    background_color = (255, 255, 255)  # Background color (RGB)
    image_width = 800  # Width of the output image
    image_height = 600  # Height of the output image

    text_to_image(text_path, output_image_path, font_size, text_color, background_color, image_width, image_height)
