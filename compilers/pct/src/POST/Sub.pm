class POST::Sub is POST::Node;

our @paramfmt;
INIT {
    @paramfmt[0] := "    .param pmc %0\n";
    @paramfmt[1] := "    .param pmc %0 :optional\n    .param int has_%0 :opt_flag\n";
    @paramfmt[2] := "    .param pmc %0 :slurpy\n";
    @paramfmt[4] := "    .param pmc %0 :named(%1)\n";
    @paramfmt[5] := "    .param pmc %0 :optional :named(%1)\n    .param int has_%0 :opt_flag\n";
    @paramfmt[6] := "    .param pmc %0 :slurpy :named\n";
    @paramfmt[8] := "    .param pmc %0 :call_sig\n";
}

=item C<symtable>
Get whole symtable. Used in register allocator.

our method symtable() {
    self<symtable>;
}

=item C<symbol($name, $value?)
Get or set variable used in POST::Sub.

our method symbol($name, $value?) {
    my %symtable := self<symtable>;
    unless %symtable {
        self<symtable> := hash();
        %symtable := self<symtable>;
    }

    if $value {
        %symtable{$name} := $value;
    }

    %symtable{$name};
}

=item C<labels>
Get all labels.

our method labels() {
    self<labels>;
}

=item C<symbol($name, $value?)
Get or set variable used in POST::Sub.

our method label($name, $value?) {
    my %labels := self<labels>;
    unless %labels {
        self<labels> := hash();
        %labels := self<labels>;
    }

    if $value {
        %labels{$name} := $value;
    }

    %labels{$name};
}


=item C<param($name, POST::Register $param)
Add Sub parameter.

our method param($name, POST::Register $param) {
    my @params := self<params>;
    unless @params {
        self<params> := list();
        @params := self<params>;
    }

    # Don't check redeclaration of register. It should be done early.

    @params.push($param);
}


=item C<constant_index>($idx?)
Get or set Sub index in PackfileConstantTable

our multi method constant_index() { self<constant_index>; }
our multi method constant_index($idx) { self<constant_index> := $idx; $idx }

=Modifiers
Various sub modifiers.

our multi method main()     { self<main> // 0 }
our multi method main($val) { self<main> := ?$val }

# XXX Conflict with PCT::Node.init.
our multi method is_init()     { self<init> // 0 }
our multi method is_init($val) { self<init> := ?$val }

our multi method load()     { self<load> // 0 }
our multi method load($val) { self<load> := ?$val }

our multi method immediate()     { self<immediate> // 0 }
our multi method immediate($val) { self<immediate> := ?$val }

our multi method postcomp()     { self<postcomp> // 0 }
our multi method postcomp($val) { self<postcomp> := ?$val }

our multi method anon()     { self<anon> // 0 }
our multi method anon($val) { self<anon> := ?$val }

our multi method is_method()     { self<method> // 0 }
our multi method is_method($val) { self<method> := ?$val }

our multi method lex()        { self<lex> // 0 }
our multi method lex($val)    { self<lex> := ?$val }

our multi method nsentry()     { self<nsentry> }
our multi method nsentry($val) { self<nsentry> := $val }

our multi method vtable()     { self<vtable> // -1 }
our multi method vtable($val) { self<vtable> := $val; $val }

our multi method blocktype()     { self<blocktype> }
our multi method blocktype($val) { self<blocktype> := $val }

our multi method namespace()     { self<namespace> }
our multi method namespace($val) { self<namespace> := $val }

our multi method hll()     { self<hll> }
our multi method hll($val) { self<hll> := $val }

our multi method loadlibs()     { self<loadlibs> }
our multi method loadlibs($val) { self<loadlibs> := $val }

our multi method outer()     { self<outer> }
our multi method outer($val) { self<outer> := $val }

our multi method multi()     { self<multi> }
our multi method multi($val) { self<multi> := $val }

our multi method pirflags()     { self<pirflags> }
our multi method pirflags($val) { self<pirflags> := $val }

our multi method compiler()     { self<compiler> }
our multi method compiler($val) { self<compiler> := $val }

our multi method compiler_args()     { self<compiler_args> }
our multi method compiler_args($val) { self<compiler_args> := $val }

our multi method subid($value)      { self<subid> := $value }
our multi method subid() {
    my $value := self<subid>;
    unless pir::defined__ip($value) {
        $value := self.unique('post');
        self<subid> := $value;
    }
    $value;
}

our method set_flag($name, $val) { self{$name} := $val; $val }

=end

our method full_name() {
    if self<namespace> {
        self<namespace>.Str ~ self.name
    }
    else {
        self.name;
    }
};

method add_directive($line) {
    my $dlist := self<directives>;
    if pir::index__iss($dlist, $line) < 0 {
        self<directives> := ~$dlist ~ ~$line ~ "\n";
    }
}

# XXX Rewrite into NQP
method add_param($pname, *%adverbs) {
    Q:PIR {
    .local pmc pname
    .local pmc adverbs

    find_lex pname, '$pname'
    find_lex adverbs, '%adverbs'

    .local int optional, slurpy, call_sig
    .local string named
    optional = adverbs['optional']
    slurpy = adverbs['slurpy']
    named = adverbs['named']
    call_sig = adverbs['call_sig']

    .local int paramseq
    paramseq = isne optional, 0
    unless slurpy goto slurpy_done
    paramseq += 2
  slurpy_done:
    unless named goto named_done
    paramseq += 4
  named_done:
    unless call_sig goto call_sig_done
    paramseq += 8
  call_sig_done:

    .local pmc paramlist
    paramlist = self['paramlist']
    unless null paramlist goto have_paramlist
    paramlist = new 'ResizablePMCArray'
    self['paramlist'] = paramlist
  have_paramlist:

    .local pmc code
    code = paramlist[paramseq]
    unless null code goto have_code
    code = new 'StringBuilder'
    paramlist[paramseq] = code
  have_code:

    .local pmc paramfmt
    paramfmt = get_hll_global ['POST';'Sub'], '@paramfmt'
    $S0 = paramfmt[paramseq]
    named = self.'escape'(named)
    code.'append_format'($S0, pname, named)

    .return ()
    }
}

INIT {
    pir::load_bytecode('nqp-setting.pbc');
}


# vim: expandtab shiftwidth=4 ft=perl6