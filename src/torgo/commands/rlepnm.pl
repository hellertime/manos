sub encode {
  (my $str = shift) =~ s {(.)(\1*)} {length($&).$1}gse;
  return $str; }

local$/;

print "uint32_t hal[] = {\n";
my $last = undef;
my $run = 0;
foreach my $el (map{sprintf("%d", $_)}unpack("C*", <>)) {
    unless (defined $last) {
       $last = $el;
    } elsif ($last eq $el) {
       $run++;
    } else {
        $run++;
        print "$last, $run,\n";
        $last = $el;
        $run = 0;
    }
}
print "};\n";
