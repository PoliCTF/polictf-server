#!/usr/bin/perl

use Switch;
use IO::Socket;
use LWP::UserAgent;
use HTTP::Cookies;
use threads;
use threads::shared;
use Data::Dumper;
use Time::HiRes qw(time);
use LWP::ConnCache;

sub randstr {
    my $length_of_randomstring = shift;
    # the length of
    # the random string to generate

    my @chars = ('a'..'z', 'A'..'Z', '0'..'9', '_');
    my $random_string;
    foreach(1..$length_of_randomstring) {
        # rand @chars will generate a random
        # number between 0 and scalar @chars
        $random_string = $chars[rand @chars];
    }
    return $random_string;
}

sub Verify {
    $resp = shift;

    if ($resp->is_success) {
        $counter++;
    } else{
        print Dumper($resp);
    }
}

sub LoginSuccessful() {
    my $accnumber = int(rand(100)) + 1;
    Verify($ua->post(
        $BASEURL . "/login",
        {"teamname" => "team$accnumber", "password" => "pass$accnumber"}
    ));
    $loggedin = true;
}
sub LoginUnsuccessful() {
    $usr = randstr(10);
    $psw = randstr(10);
    Verify($ua->post(
        $BASEURL . "/login",
        { "teamname" => $usr, "password" => $psw }
    ));
    $loggedin = false;
}

sub Logout() {
    Verify($ua->get($BASEURL . "/logout"));
    $loggedin = false;
}

sub NotLoggedInActions() {
    if (rand(100) < 90) {
        LoginSuccessful();
    } else {
        LoginUnsuccessful();
    }
}
sub Submit() {
    Verify($ua->post($BASEURL . "/team/submit", [ id => rand(40), flag => randstr(10) ]));
}
sub LoggedInActions() {
    if (rand(100) < 10) {
        Logout();
    }
    elsif(rand(100) < 30) {
        Verify($ua->get($BASEURL . "/common/status"));
    }
    elsif(rand(100) < 30) {
        Verify($ua->get($BASEURL . "/team/status"));
    }
    elsif(rand(100) < 20) {
        Verify($ua->get($BASEURL . "/common/challenge/" . rand(40)));
    }
    else {
        Submit();
    }
}
sub OneBusyUser() {
    local $loggedin = false;
    local $ua = new LWP::UserAgent();
    my $cookie_jar = HTTP::Cookies->new;
    $cookie_jar->clear;
    $ua->cookie_jar($cookie_jar);
    $ua->conn_cache(LWP::ConnCache->new());

    while (1) {
        if ($loggedin == false) {
            NotLoggedInActions();
        } else {
            LoggedInActions();
        }
    }
}
sub Status() {
    my $start = time();
    my $end;

    while (1) {
        sleep(1);
        $ctr = $counter;
        $end = time();
        printf("Successfully executed %i requests in %.2f seconds (avg %.2f req/s)\n", $ctr, $end - $start, $ctr / ($end - $start));
    }
}

my @arr;
our $counter: shared;
our $BASEURL;
our $wait;

$counter = 0;
$BASEURL = $ARGV[1];
$arr[0] = threads->new(\&Status);

for ($i = 1; $i <= $ARGV[0]; $i++) {
    $arr[$i] = threads->new(\&OneBusyUser);
}

for ($i = 0; $i <= $ARGV[0]; $i++) {
    $arr[$i]->join;
}
