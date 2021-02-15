ir_image_sub = rossubscriber('/flirone/images/ir_16b');
rgb_image_sub = rossubscriber('/flirone/images/rgb_jpg');

pause(2);

figure(1);
figure(2);

while(1)
    rgb_image_data = receive(rgb_image_sub, 15);
    rgb_image = readImage(rgb_image_data);
    ir_image_data = receive(ir_image_sub, 15);
    ir_image = readImage(ir_image_data);
    
    % remove last 2 rows of IR image as they are broken
    ir_image = ir_image(1:end-2,:);
    
    % Get max value and location
    [max_in_each_row, max_index_in_each_row] = max(ir_image);
    [max_ir, max_row] = max(max_in_each_row);
    max_col = max_index_in_each_row(max_row);
    
    % Get min value and location
    [min_in_each_row, min_index_in_each_row] = min(ir_image);
    [min_ir, min_row] = min(min_in_each_row);
    min_col = min_index_in_each_row(min_row);
    
    % Scale image from min to max, 8 bit
    pixel_range = max_ir - min_ir;
    scaled_ir = (double(ir_image) - double(min_ir)) ./ double(pixel_range);
    
    % Adjust image sizes to match
    scaled_ir = imresize(scaled_ir, 4);
    scaled_rgb = imresize(rgb_image, 0.5);
    
    % Display figures
    figure(1);
    imshow(scaled_ir);
    figure(2);
    imshow(scaled_rgb);
end