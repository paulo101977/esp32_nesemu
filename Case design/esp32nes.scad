$fn=80;
// Valid options are top/bottom/both
//outputModel="top";
outputModel="bottom";
//outputModel="both";
// The shell is transparent for top half or whole device in preview mode
transparentShell = $preview && outputModel != "bottom";
shellSeamHeight = -13.1;
supportHoles = [[29.5, 31.5],[31.5,5.5], [42, 17.25], [46.75, 10], [55.25, 6.5], [87.5, 5.5], [90, 31.5]];
buttons=[[20,9,240],[20,31,-60],[7,20,-30],[33,20,30]];
dpad_buttons = [[91,19,0],[111,19,180],[101,9,90],[101,29,-90]];
        lcd_board_internal_protrusion = 3;
       lcd_board_width = 86;
//        lcd_board_thickness = 2;
        lcd_board_thickness = 2;
        lcd_board_thickness_extra = lcd_board_thickness+lcd_board_internal_protrusion;
        lcd_width = 70;
        lcd_thickness = 2;
        lcd_offset = 6;
        lcd_height = 52;
        lcd_hole_radius = 1.75;
        sd_width = 26.5;
//        sd_height = 18;
        sd_height = 28;
        sd_thickness = 3.9;


module lcd_supports(rad, height) {
            translate([-lcd_board_width/2 + 3, lcd_height/2 - 3,0])
            cylinder(r=rad, h = height + 0.1, center=true);

            translate([-lcd_board_width/2 + 3, -lcd_height/2 + 3,0])
            cylinder(r=rad, h = height + 0.1, center=true);

            translate([lcd_board_width/2 - 7, lcd_height/2 - 3, 0])
            cylinder(r=rad, h = height + 0.1, center=true);

            translate([lcd_board_width/2 - 7, -lcd_height/2 + 3,0])
            cylinder(r=rad, h = height + 0.1, center=true);
}    

module contents() {
    module cpu() {
        color("green")
        cube([60, 20.5, 2.5]);
        
        color("silver")
        translate([6, 5.5, 2.5])
        cube([19.5, 11, 2.5]);
        
    }
    
    module controller() {
        
        module directionButton() {
            linear_extrude(height = 12)
                hull(){
                    square([7,10], center=true);
                    translate([6,0])
                    circle(r=2);
                }
        }

        button_radius = 5.85;
        button_height = 12;
        
        color("green")
        linear_extrude(height = 1.5, center = true)
        difference() {
        polygon(points=[
            [0,11], [9.5,11], [9.5,0], [89.75,0], [111.0, 0], [111.0, 11], [119, 11],
            [119.5, 39.5], [90, 39.5], [86, 34.5], [34, 34.5], [30, 39.5], [0, 39.5], [0, 11]
        ]);
            
        for (loc=supportHoles)
            translate(loc)
            circle(r=1.6);
        }
        
        color("gray") {
            // Right buttons
            for(loc=buttons) {
                translate([loc[0],loc[1],button_height/2])
                cylinder(r = button_radius, h = button_height, center=true);
            }
            
            
            // D-Pad
            for (loc=dpad_buttons) {
                translate([loc[0], loc[1],0])
                rotate([0,0,loc[2]])
                directionButton();
            }
            
            //Select
            translate([65,26,0])
            cube([8, 5.1, button_height]);
            
            //Start
            translate([54.5,25.5,0])
            linear_extrude(height=button_height)
                hull() {
                    square([0.1, 6]);
                    
                    translate([-7,3])
                    circle(r=1.25);
                }
        }
    }
        
        
    module LCD() {    
         
        // board
        color("red") {
        difference() {
            cube([lcd_board_width, lcd_height, lcd_board_thickness_extra], center=true);
            
            lcd_supports(lcd_hole_radius, lcd_board_thickness_extra);
        }
        
        // Add raised areas to make room for soldered header
        translate([lcd_board_width/2-1,0,1+lcd_board_internal_protrusion/2]) rotate([90,0,0]) cylinder(r=0.6, h=lcd_height * 0.75, center=true);

        translate([-lcd_board_width/2+2,0,1+lcd_board_internal_protrusion/2]) rotate([90,0,0]) cylinder(r=0.6, h=lcd_height * 0.75, center=true);
        }
        
        // lcd
        color("black")
        translate([-lcd_board_width/2 + lcd_width/2 + lcd_offset, 0, lcd_board_thickness_extra/2 + lcd_thickness/2])
        cube([lcd_width, lcd_height, lcd_thickness], center=true);
        
        // sd card
        color("silver")
//        translate([0, -height/2 + sd_height/2, -(sd_thickness+board_thickness)/2])
        translate([-5, -lcd_height/2 + sd_height/2 - 5, -(sd_thickness+lcd_board_thickness_extra)/2 + lcd_board_internal_protrusion-3])
        cube([sd_width, sd_height, sd_thickness], center=true);
    }

    
    //------------------------------
    
    rotate([0,0,180])
    translate([0,0,7.25 - lcd_board_internal_protrusion/2])
    LCD();
    
    rotate([0,0,180])
    translate([-30, 0, 1.5])
    cpu();
    
    translate([60, -24, 0])
    rotate([180,180,0])
    controller();
}


    module shellOuter() {
        //------------------------------

        hull() {
            translate([0,41.5,3])
            minkowski() {
                cube([90, 0.1,4], center=true);
                sphere(r=2);
            }
            
            translate([-48,32,-2])
            sphere(r=9);
            
            translate([48,32,-2])
            sphere(r=9);
            
            translate([-57,-42,-4])
            sphere(r=11);
                            
            translate([57,-42,-4])
            sphere(r=11);    
        }    
    }
    
 
    module shellInner() {
        module controllerSupport() {
            linear_extrude(height=12) {
                square([0.8,24], center=true);

                square([24,0.8], center=true);
                
                difference() {
                    circle(r=12.8);
                    circle(r=12);
                }
                
                difference() {
                    circle(r=6.8);
                    circle(r=6);
                }        
            }    
        }
        
        module controllerScrewHoles() {
            rotate([0,0,180])
            for (loc=supportHoles) {
                translate(loc)
                difference() {
                    cylinder(r=2, h=9.9);
                    translate([0,0,-0.1])
                    cylinder(r=1.2, h=11);
                }
            }
        }
        
        module dpadSupports() {
            a1=0;
            a2=5.5;
            a3=7;
            a4=14;
            a5=15.5;
            a6=16;
            p=[
                [-a6,a1],[-a5,-a2],[-a4,-a3],
                [-a3,-a3],[-a3,-a4],[-a2,-a5],
                [a1, -a6], [a2, -a5], [a3, -a4],
                [a3, -a3],[a4, -a3],[a5, -a2],
                [a6, a1],[a5,a2],[a4,a3],
                [a3,a3],[a3,a4],[a2,a5],
                [a1,a6],[-a2,a5],[-a3,a4],
                [-a3,a3],[-a4,a3],[-a5,a2]
            ];
            
            difference() {
                linear_extrude(height=7)
                difference() {
                    scale(1)
                    polygon(points=p);

                    scale(0.94)
                    polygon(points=p);

                }
            
                translate([0,0,1.5]) {
                    rotate([0,0,45])
                    cube([30,2.25,5], center=true);

                    rotate([0,0,-45])
                    cube([30,2.25,5], center=true);
                }               
            }
        
        }



        module buttonSupports() {
            for (loc=buttons) {
                difference() {
                    translate([loc[0],loc[1],0])
                    cylinder(r=6.2,h=7);

                    translate([loc[0],loc[1],0]) {
                        translate([0,0,-0.1])
                        cylinder(r=5.8,h=7.2);
                        
                        rotate([0,0,loc[2]])
                        translate([-1,-1,-0.1])
                        cube([2.25,10,4.1]);
                    }
                          
                    translate([20,20,2]) {
                        cube([4,12,4.1], center=true);
                        cube([2.5,40,4.1], center=true);
                        cube([16,4,4.1], center=true);
                        cube([40,2.5,4.1], center=true);
                    }
                }
            }
        }
 
       
        module screenSupports() {
//            w1=2.5;
            w1=lcd_hole_radius;
            w2=0.8;
            
            translate([0,0,0.6])
            scale([1,1,1.75])
            difference() {
                lcd_supports(w1, lcd_board_thickness);
                scale([1,1,1.1])
                lcd_supports(w2, lcd_board_thickness);
            }
        }
        //------------------------------
        controllerOffset=[-41,-27,-15];

        translate(controllerOffset)
        controllerSupport();
        
        translate([-controllerOffset[0], controllerOffset[1], controllerOffset[2]])
        controllerSupport();    
        
        translate([60,-8,-3])
        controllerScrewHoles();
        
        translate([controllerOffset[0],controllerOffset[1],-0.1])
        dpadSupports();
        
        translate([20,-48,-0.1])
        buttonSupports();
        
        translate([4,16,5])
        screenSupports();
        
        screwHoleWalls();
    }

module shell() {
    //------------------------------

    if (transparentShell) {
        %shellOuter();
        shellInner();
    } else {
        difference() {
            shellOuter();
            
            scale([0.975, 0.97, 0.8])
            shellOuter();
        }
        intersection() {
            shellInner();
            shellOuter();
        }
    }

    
}

holeDepth = 13;
module screwHoles() {
    // These are the openings on the back meant to be cut out of the shell
    translate([60,-8, -10])
    rotate([0,0,180])
    for (loc=supportHoles) {
        translate(loc)
        cylinder(r=2.5, h=holeDepth, center=true);
    }
}

module screwHoleWalls() {
    // These are the inside parts of the screw holes that must be added back
    // prior to slicing the model

    translate([60,-8,-3 - holeDepth/2])
    rotate([0,0,180])
    for (loc=supportHoles) {
        translate(loc)
        difference() {
            cylinder(r=3.25, h=holeDepth, center=true);
            translate([0,0,-0.1])
            cylinder(r=1.2, h=holeDepth+0.5, center=true);
            translate([0,0,-0.75])
            cylinder(r=2.5, h=holeDepth+0.5, center=true);
        }
    }
    
}


module main() {
    if ($preview && outputModel == "top") {
        shell();
        
        translate([0,16,-2.5])
        contents();
    } else {
        difference() {
            shell();

            translate([0,16,-2.5])
            contents();
            
            screwHoles();
        }
    }
}

jointHeight=1.5;
module joint(inner=1, outer = 1) {
    if (!transparentShell) {
        intersection() {
            translate([0,0,shellSeamHeight/2+jointHeight+2.75])
            cube([150,150,jointHeight], center=true);

            difference() {
                scale([0.987 * outer, 0.985 * outer, 0.9 * outer])
                shellOuter();
                
                translate([0,0,0.01])
                scale([0.97 * inner, 0.97 * inner, 0.8 * inner])
                shellOuter();

            }
        }
    }
}

if (outputModel == "top") {
    difference() {
        main();
        translate([0,0,shellSeamHeight])
        cube([150,150,20], center=true);
        color("gray")
        translate([0,0,-0.1])
        joint(inner=0.99);
    }
} else if (outputModel == "bottom") {
    intersection() {
        main();
        translate([0,0,shellSeamHeight])
        cube([150,150,20], center=true);
    }
    color("gray")
    difference(){
        joint();
        translate([0,16,-2.5])
        contents();
    }
} else {
    main();
}
