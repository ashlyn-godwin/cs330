# cs330
Computer Graphics And Visualization


    How do I approach designing software?
    - When I design software, I approach the design with readability, portability, and functionality in mind. First of all, the code that is written should be easy to discern and edit. This means that adequate documentation must be provided, and that the code is well commented and readable. Likewise, the code should be written in a way that lets it be used for purposes different than originally intended. The code should be able to be adapted to changes in design philosophy and changes in required functionality. Finally, the software should meet all of the functional requirements that are important to have it work.
    
        What new design skills has your work on the project helped you to craft?
        - Work on this project has taught me a lot about rendering loops and smarter ways to organize object data. Originally I had multiple objects declared: one for each shape in the scene. Doing this ended up with a lot of duplicate data that slowed the runtime immensely. I learned, however, that the way rendering works does not require these multiple objects. Instead I was able to have one object called meshes and have it render multiple times for all of the shapes in the scene which increased performance.
        
        What design process did you follow for your project work?
        - I had several different functions which all performed different roles in the rendering process. Some performed callback functions for keyboard and mouse controls, while others allowed for easier shader creation. All of these allowed for the complicated process of rendering 3D graphics to be much simpler. 
        
        How could tactics from your design approach be applied in future work?
        - The main takeaway I have from this project is using functions to simplify repetitive actions when utilizing an API. OpenGL has a lot of complicated processes that need to occur every time an object is rendered on screen. One of the biggest flaws in my project has been duplication of work. If given more time, I would like to have a gameObject class that abstracts a lot of the rendering pipeline to avoid duplicating work. 

    How do I approach developing programs?
        What new development strategies did you use while working on your 3D scene?
        - The main new strategy I tried was writing down all of the functional pieces of the program and then sorting them by what they did. An example of this would be the uniforms for the shaders used in the final project. I wrote on paper all of the uniforms and whether or not they were universal for all shapes or individualized. After this, I was able to structure my code in the Render function in order to clearly identify what to change for each object. 
        How did iteration factor into your development?
        -Iteration was incredibly important to developing this project. All of the milestones built upon the functionality of the last which allowed for a lot of the rendering process to be completed piece by piece. 
        How has your approach to developing code evolved throughout the milestones, which led you to the project’s completion?
        - My approach to code at the beginning of the course involved working through the tutorials and then adapting that code for the milestones. While that worked for the first few weeks of the course, overtime the code began to become unreadable and difficult to maintain. To solve this, I restructured the code so all of the uniforms that were universal were declared once and the remaining uniforms were redefined for each object. This allowed for a much more structured system. 
        
    How can computer science help me in reaching my goals?
        How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?
        - Having an understanding of computer graphics and visualizations gave me several useful skills. For one, I now know how computers display 3D images, which would be useful in any career related to game design or 3D visualization. More importantly, OpenGL was a very good example for practicing working with APIs. My experience with API usage was fairly limited before this class so getting the extra practice building off and using OpenGL will be extremely beneficial for my future educational path. 
