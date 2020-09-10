#import "RevRootViewController.h"
#import "DetailViewController.h"
#include "SearchKit.h"
#include "machstuff.h"


@interface RevRootViewController ()

@property (nonatomic, strong) NSMutableArray * objects;
@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) UIAlertController *pidAlert;
@property (strong, nonatomic) UIAlertController *aboutAlert;
@property (strong, nonatomic) UIBarButtonItem *pidButton;
@property (strong, nonatomic) UIBarButtonItem *aboutButton;
@property (strong, nonatomic) UILabel *pidLabel;
@property int pid;
@property result_t resultnum;
@property mach_port_t task;
@property NSInteger width;
@property NSInteger height;
@property NSInteger fheight;
@property (strong, nonatomic) UIColor *textcolor;
@property (strong, nonatomic) UIColor *windowcolor;
@property (strong, nonatomic) UISwitch *byteswitch;
@property (strong, nonatomic) UILabel *byteswitchlabel;
@property (strong, nonatomic) UISwitch *taskpause;
@property (strong, nonatomic) UILabel *taskpauselabel;
@property (strong, nonatomic) UILabel *searchinput;
@property NSString *inputstring;
@property (strong, nonatomic) UIButton *searchbutton;

@end

int __isOSVersionAtLeast(int major, int minor, int patch) {
    NSOperatingSystemVersion version;
    version.majorVersion = major;
    version.minorVersion = minor;
    version.patchVersion = patch;
    return [[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:version];
}

@implementation RevRootViewController

- (void)viewDidLoad {
    [super viewDidLoad];
	 _objects = [NSMutableArray array];


    self.inputstring = @"??";

    CGRect screen = [[UIScreen mainScreen] bounds];
    self.width = (NSInteger) CGRectGetWidth(screen);
    self.height = (NSInteger) CGRectGetHeight(screen);
    //self.fheight = (NSInteger) self.navigationController.navigationBar.frame.size.height;
    self.fheight = 0;


    UIBarButtonItem *controlsButton = [[UIBarButtonItem alloc] initWithTitle:@"Controls" style:UIBarButtonItemStylePlain target:self action:@selector(makeControlsVisible:)];

    self.pidButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(showMyAlert:)];
    self.aboutButton = [[UIBarButtonItem alloc] initWithTitle:@"About" style:UIBarButtonItemStylePlain target:self action:@selector(showMyAlert:)];

    self.navigationItem.rightBarButtonItem = self.aboutButton;
    self.navigationItem.leftBarButtonItem = controlsButton;
    self.detailViewController = (DetailViewController *)[[self.splitViewController.viewControllers lastObject] topViewController];

    self.title = @"RevelariOS";

    self.pidAlert = [UIAlertController alertControllerWithTitle:@"Add PID" message:@"Enter Process ID and Bytes / String for RevelariOS to search with. Click the 'About' button in the main view for how to find your process ID.\n\nIf you want to search for the STRING 'ABCD' in bytes, enter '41424344'. If you want to search for a stored number such as 123456, turn it to hex (0x1E240) and enter it in Little Endian (40E201). If you want to search a string in memory, such as 'Leaderboards', type it as it is. Your input would simply be Leaderboards." preferredStyle:UIAlertControllerStyleAlert];
    UIAlertAction *pidAlertOk = [UIAlertAction actionWithTitle:@"Add" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {
        self.inputstring = [[self.pidAlert textFields][1] text];
        if ([self.byteswitch isOn]) {
            [self.searchinput setText:[NSString stringWithFormat:@"Search String: %@", self.inputstring]];
        }
        else {
            [self.searchinput setText:[NSString stringWithFormat:@"Search Bytes: %@", self.inputstring]];
        }
        if ([[self.pidAlert textFields][0] hasText]) {
            self.pid = [[[self.pidAlert textFields][0] text] intValue]; // ORIGINAL
            self.task = get_tfp(self.pid); //ORIGINAL
            if (self.task != MACH_PORT_NULL) {
                NSString *pidform = [NSString stringWithFormat:@"Current PID: %i", self.pid];
                [self.pidLabel setText:pidform];
            }
            else {
                UIAlertController *failalert = [UIAlertController alertControllerWithTitle:@"ERROR" message:[NSString stringWithFormat:@"task_for_pid(%i) failure!\n\nEither you don't have correct entitlements for RevelariOS, are trying to obtain task_for_pid(0), or inputted an invalid process ID", self.pid] preferredStyle:UIAlertControllerStyleAlert];
                UIAlertAction *tfpAlertDismiss = [UIAlertAction actionWithTitle:@"Dismiss" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {
                    [self.window makeKeyAndVisible];
                }];
                [failalert addAction:tfpAlertDismiss];
                [self presentViewController:failalert animated:YES completion:nil];
            }
        }
    }];

    [self.pidAlert addTextFieldWithConfigurationHandler:^(UITextField *textField) {
        textField.placeholder = @"PID";
        textField.clearButtonMode = UITextFieldViewModeWhileEditing;
        textField.keyboardType = UIKeyboardTypeNumberPad;
    }];
    [self.pidAlert addTextFieldWithConfigurationHandler:^(UITextField *byteField) {
        byteField.placeholder = @"Bytes / String Input";
        //textField.textColor = [UIColor blueColor];
        byteField.clearButtonMode = UITextFieldViewModeWhileEditing;
        //textField.borderStyle = UITextBorderStyleRoundedRect;
    }];
    [self.pidAlert addAction:pidAlertOk];

    NSString *aboutMessage = @"RevelariOS is a DEVELOPER program that searches for bytes or strings inside of memory and prints out the results. This is useful for patching binaries and items inside of memory.\n\nTo find a process ID, type 'ps ax' inside of NewTerm or an equivalent application and enter your desired process ID under 'RevelariOS Controls'";
    self.aboutAlert = [UIAlertController alertControllerWithTitle:@"What is RevelariOS?" message:aboutMessage preferredStyle:UIAlertControllerStyleAlert];
    UIAlertAction *aboutAlertOk = [UIAlertAction actionWithTitle:@"Got it!" style:UIAlertActionStyleDefault handler:nil];
    [self.aboutAlert addAction:aboutAlertOk];

    self.window = [[UIWindow alloc] init];
    [self.window setFrame:CGRectMake(0, self.fheight, self.width, self.height)];
    [self.navigationController.navigationBar addSubview:self.window];
    [self.view addSubview:self.window];
    [self.window makeKeyWindow];
    [self.window setBackgroundColor:[UIColor whiteColor]];
    [self.window setHidden:YES];

    self.pidLabel = [[UILabel alloc] initWithFrame:CGRectMake(self.width/2-100, 3, 200, 50)];
    [self.pidLabel setText:@"Current PID:"];
    [self.pidLabel setTextAlignment:NSTextAlignmentCenter];
    [self.window addSubview:self.pidLabel];

    self.byteswitch = [[UISwitch alloc] initWithFrame:CGRectMake(self.width*0.75, 83, 50, 30)];
    [self.byteswitch setOn:NO];
    [self.byteswitch addTarget:self action:@selector(byteSwitchChanged:) forControlEvents:UIControlEventValueChanged];
    [self.window addSubview:self.byteswitch];


    //BYTE SWITCH TO STRING
    self.byteswitchlabel = [[UILabel alloc] initWithFrame:CGRectMake(self.width*0.25-75, 83, 150, 30)];
    [self.byteswitchlabel setText:@"Search For String"];
    [self.window addSubview:self.byteswitchlabel];

    self.taskpause = [[UISwitch alloc] initWithFrame:CGRectMake(self.width*0.75, 163, 50, 30)];
    [self.taskpause setOn:NO];
    [self.taskpause addTarget:self action:@selector(taskSwitchChanged:) forControlEvents:UIControlEventValueChanged];
    [self.window addSubview:self.taskpause];

    self.taskpauselabel = [[UILabel alloc] initWithFrame:CGRectMake(self.width*0.25-75, 163, 150, 30)];
    [self.taskpauselabel setText:@"Pause Task"];
    [self.window addSubview:self.taskpauselabel];

    //SEARCH BYTES
    self.searchinput = [[UILabel alloc] initWithFrame:CGRectMake(self.width*0.25 - self.width*0.125, 33, self.width*0.75, 50)];
    [self.searchinput setText:[NSString stringWithFormat:@"Search Bytes: %@", self.inputstring]];
    [self.searchinput setTextAlignment:NSTextAlignmentCenter];
    [self.window addSubview:self.searchinput];

    self.searchbutton = [[UIButton alloc]initWithFrame:CGRectMake(self.width/2-75, 123, 150, 50)];
    [self.searchbutton setTitleColor:[UIColor blueColor] forState:UIControlStateNormal];
    [self.searchbutton setTitle:@"Search" forState:UIControlStateNormal];
    [self.searchbutton setTitleColor:[UIColor whiteColor] forState:UIControlStateSelected];
    [self.searchbutton setTitle:@"Searching..." forState:UIControlStateSelected];
    [self.searchbutton addTarget:self action:@selector(searchButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.window addSubview:self.searchbutton];

}

- (void)viewWillAppear:(BOOL)animated {
    self.clearsSelectionOnViewWillAppear = self.splitViewController.isCollapsed;
    [super viewWillAppear:animated];
}


-(NSString*) searchTypeToString:(search_t)sret {
	switch (sret) {
	case 0:
		return @"Search Success";
	case 1:
		return @"Generic Search Failure";
	case 2:
		return @"Input Bytes Uneven: If you want to put in 0xab3 put in 0AB3 instead of AB3";
	case 3:
		return @"Input Bytes Too Large: Keep the input bytes under 100 characters";
	case 4:
		return @"Write Success";
	case 5:
		return @"Generic Write Failure";
	case 6:
		return @"Attempting to write to bad address";
	}
	return @"Unknown Error";
}

-(void) searchButtonPressed:(id)sender {
    vm_address_t start;
    vm_address_t end;
    get_region_size(self.task, (vm_address_t*) &start, (vm_address_t*) &end);

    bool isString = [self.byteswitch isOn];
    vm_address_t outaddr[SEARCH_MAX];
    char *in = malloc(sizeof(char)*100);
    strcpy(in, [self.inputstring UTF8String]);


	for (int i=0; i<self.resultnum; i++) {
		[_objects removeObjectAtIndex:0];
		[self.tableView deleteRowsAtIndexPaths: @[ [NSIndexPath indexPathForRow:0 inSection:0] ] withRowAnimation:UITableViewRowAnimationAutomatic];
	}

    search_t sret;
    sret = search_data(self.task, isString, false, (vm_address_t) start, (vm_address_t) end, (vm_address_t**) &outaddr, &_resultnum, in);

    if (sret != SEARCH_SUCCESS) {
        UIAlertController *erroralert = [UIAlertController alertControllerWithTitle:@"RevelariOS Error" message:[NSString stringWithFormat:@"Search Error:\n\n%@", [self searchTypeToString:sret]] preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *aboutAlertOk = [UIAlertAction actionWithTitle:@"Ok" style:UIAlertActionStyleDefault handler:nil];
        [erroralert addAction:aboutAlertOk];
        [self presentViewController:erroralert animated:YES completion:nil];
        [self.window setWindowLevel:[self.window windowLevel] -1];
    }
    else {
        UIAlertController *successalert = [UIAlertController alertControllerWithTitle:@"RevelariOS Search" message:[NSString stringWithFormat:@"Total Found: %i", self.resultnum] preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *aboutAlertOk = [UIAlertAction actionWithTitle:@"Ok" style:UIAlertActionStyleDefault handler:nil];
        [successalert addAction:aboutAlertOk];
        [self presentViewController:successalert animated:YES completion:nil];
        [self.window setWindowLevel:[self.window windowLevel] -1];
    }

    for (int i=self.resultnum; i>0; i--) {
        [_objects insertObject:[NSString stringWithFormat:@"0x%lx", (unsigned long) outaddr[i]] atIndex:0];
        [self.tableView insertRowsAtIndexPaths:@[ [NSIndexPath indexPathForRow:0 inSection:0] ] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

-(void) byteSwitchChanged:(UISwitch*)paramSender {
    if ([paramSender isOn]) {
        [self.searchinput setText:[NSString stringWithFormat:@"Search String: %@", self.inputstring]];
    }
    else {
        [self.searchinput setText:[NSString stringWithFormat:@"Search Bytes: %@", self.inputstring]];
    }
}

-(void) taskSwitchChanged:(UISwitch *) paramSender {
    if (self.task != MACH_PORT_NULL) {
        if ([paramSender isOn]) {
            task_suspend(self.task);
        }
        else {
            task_resume(self.task);
        }
    }
    else {
        [paramSender setOn:NO];
    }
}

- (void) makeControlsVisible:(id)sender {

    if (@available(iOS 13.0, *)) {
        if (self.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark) {
            self.textcolor = [UIColor whiteColor];
            self.windowcolor = [UIColor blackColor];
            [self.byteswitchlabel setTextColor:self.textcolor];
            [self.taskpauselabel setTextColor:self.textcolor];
            [self.window setBackgroundColor:self.windowcolor];
            [self.pidLabel setTextColor:self.textcolor];
        }
        else {
            self.textcolor = [UIColor blackColor];
            self.windowcolor = [UIColor whiteColor];
            [self.byteswitchlabel setTextColor:self.textcolor];
            [self.taskpauselabel setTextColor:self.textcolor];
            [self.window setBackgroundColor:self.windowcolor];
            [self.pidLabel setTextColor:self.textcolor];
        }
    }
    else {
        self.textcolor = [UIColor blackColor];
        self.windowcolor = [UIColor whiteColor];
        [self.byteswitchlabel setTextColor:self.textcolor];
        [self.taskpauselabel setTextColor:self.textcolor];
        [self.window setBackgroundColor:self.windowcolor];
        [self.pidLabel setTextColor:self.textcolor];
    }
    [self.window setFrame:CGRectMake(0, self.fheight, self.width, self.height)];

    if ([self.window isHidden]) {
        [self.view bringSubviewToFront:self.window];
        self.navigationItem.rightBarButtonItem = self.pidButton;
        [self.window setHidden:NO];
        self.title = @"RevelariOS Controls";
        [sender setTitle:@"Done"];
    }
    else {
        [self.window setHidden:YES];
        self.navigationItem.rightBarButtonItem = self.aboutButton;
        self.title = @"RevelariOS";
        [sender setTitle:@"Controls"];
    }
}

- (void)showMyAlert:(id)sender {
    if ([self.window isHidden]) {
        [self presentViewController:self.aboutAlert animated:YES completion:nil];
    }
    else {
        [self presentViewController:self.pidAlert animated:YES completion:nil];
        [self.window setWindowLevel:[self.window windowLevel] -1];
    }
}


#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"showDetail"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        NSDate *object = self.objects[indexPath.row];
        DetailViewController *controller = (DetailViewController *)[[segue destinationViewController] topViewController];
        controller.detailItem = object;
        controller.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
        controller.navigationItem.leftItemsSupplementBackButton = YES;
        self.detailViewController = controller;
    }
}


#pragma mark - Table View Data Source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	return _objects.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	static NSString *CellIdentifier = @"Cell";
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

	if (!cell) {
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
	}

	NSString *addr = _objects[indexPath.row];
	//cell.textLabel.text = date.description;
	cell.textLabel.text = addr;
	return cell;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
	[_objects removeObjectAtIndex:indexPath.row];
	[tableView deleteRowsAtIndexPaths:@[ indexPath ] withRowAnimation:UITableViewRowAnimationAutomatic];
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {

	UIAlertController *writeAlert = [UIAlertController alertControllerWithTitle:[NSString stringWithFormat:@"Write Data"] message:@"Enter the data you would like to write in bytes.\n\nFor example, to write 123456 enter 01E240 (0x1E240)" preferredStyle:UIAlertControllerStyleAlert];

    UIAlertAction *actionLeave = [UIAlertAction actionWithTitle:@"Dismiss" style:UIAlertActionStyleDefault handler:nil];
    UIAlertAction *actionOk = [UIAlertAction actionWithTitle:@"Write" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {
			NSString *input = [[writeAlert textFields][0] text];
			unsigned long long addy = 0;
			UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
			NSScanner* scanner = [NSScanner scannerWithString:cell.textLabel.text];
			[scanner scanHexLongLong:&addy];
			char *in = malloc(sizeof(char)*100);
   			strcpy(in, [input UTF8String]);

    		search_t sret;
    		sret = write_data(self.task, false, (vm_address_t) addy, in);
    		if (sret != WRITE_SUCCESS) {
                UIAlertController *erroralert = [UIAlertController alertControllerWithTitle:@"RevelariOS Error" message:[NSString stringWithFormat:@"Write Error:\n\n%@", [self searchTypeToString:sret]] preferredStyle:UIAlertControllerStyleAlert];
                UIAlertAction *aboutAlertOk = [UIAlertAction actionWithTitle:@"Ok" style:UIAlertActionStyleDefault handler:nil];
                [erroralert addAction:aboutAlertOk];
        		[self presentViewController:erroralert animated:YES completion:nil];
        		[self.window setWindowLevel:[self.window windowLevel] -1];
    		}
   	 		else {
                UIAlertController *successalert = [UIAlertController alertControllerWithTitle:@"RevelariOS Write" message:[NSString stringWithFormat:@"Write Success!\n\nWrote 0x%@ to address %@", [[writeAlert textFields][0] text], cell.textLabel.text] preferredStyle:UIAlertControllerStyleAlert];
                UIAlertAction *aboutAlertOk = [UIAlertAction actionWithTitle:@"Ok" style:UIAlertActionStyleDefault handler:nil];
                [successalert addAction:aboutAlertOk];
        		[self presentViewController:successalert animated:YES completion:nil];
        		[self.window setWindowLevel:[self.window windowLevel] -1];
    		}

        }];
    [writeAlert addTextFieldWithConfigurationHandler:^(UITextField *byteField) {
        byteField.placeholder = @"Ex: 01E240";
        byteField.clearButtonMode = UITextFieldViewModeWhileEditing;
    }];
	[writeAlert addAction:actionLeave];
    [writeAlert addAction:actionOk];
    [self presentViewController:writeAlert animated:YES completion:nil];

	//[tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
