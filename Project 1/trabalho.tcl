if {$argc == 2} {
	set opcao [lindex $argv 0]
	set protocolo [lindex $argv 1]
	set window 20
} elseif {$argc == 3} {
	set opcao [lindex $argv 0]
	set protocolo [lindex $argv 1]
	set window [lindex $argv 2]
} else {
	puts "Argumentos: Cenario(1/2) Protocolo(udp/tcp) Janela"
	exit 1
}

set ns [new Simulator]

$ns color 1 Blue
$ns color 2 Red
$ns color 3 Green

# create nam trace file
set nf [open out.nam w]
$ns namtrace-all $nf

# create trace file
set nt [open out.tr w]
$ns trace-all $nt

proc fim {} {
	global ns nf
	global ns nt
	$ns flush-trace
	close $nf
	close $nt
	exec nam out.nam
	exit 0
}

#Nos
set n0 [$ns node] 
set n1 [$ns node] 
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node] 
set n5 [$ns node] 
set n6 [$ns node]
set n7 [$ns node]

#nomes
$ns at 0.0 "$n0 label PC_A"
$ns at 0.0 "$n1 label PC_B"
$ns at 0.0 "$n2 label PC_C"
$ns at 0.0 "$n5 label PC_D"
$ns at 0.0 "$n7 label PC_E"

#linkar
$ns duplex-link $n0 $n1 10Mb 10ms DropTail 
$ns duplex-link $n1 $n2 10Mb 10ms DropTail 
$ns duplex-link $n2 $n3 10Mb 10ms DropTail 
$ns duplex-link $n2 $n5 10Mb 10ms DropTail 
$ns duplex-link $n3 $n6 10Mb 10ms DropTail 
$ns duplex-link $n5 $n6 10Mb 10ms DropTail 
$ns duplex-link $n4 $n5 10Mb 10ms DropTail 
$ns duplex-link $n5 $n7 10Mb 10ms DropTail 
$ns simplex-link $n4 $n1 10Mb 5ms DropTail 

#alteracao do tamanho da fila
#2097152 bytes / 1000 bytes = 2098 pacotes
$ns queue-limit $n0 $n1 2098
$ns queue-limit $n1 $n2 50 
$ns queue-limit $n2 $n3 50  
$ns queue-limit $n2 $n5 50 
$ns queue-limit $n3 $n6 50  
$ns queue-limit $n5 $n6 50  
$ns queue-limit $n4 $n5 50  
$ns queue-limit $n5 $n7 50 
$ns queue-limit $n4 $n1 50 

#organizar
$ns duplex-link-op $n0 $n1 orient right
$ns duplex-link-op $n1 $n2 orient right
$ns duplex-link-op $n2 $n3 orient right
$ns duplex-link-op $n2 $n5 orient down
$ns duplex-link-op $n3 $n6 orient down
$ns duplex-link-op $n5 $n6 orient right
$ns duplex-link-op $n4 $n5 orient right
$ns duplex-link-op $n5 $n7 orient down
$ns simplex-link-op $n4 $n1 orient up

#formas
$n0 shape hexagon
$n1 shape square
$n5 shape square
$n7 shape hexagon

#cores
$n0 color blue
$n1 color red
$n5 color green
$n7 color blue

#cbr0
set cbr0 [new Application/Traffic/CBR]
#2 MB = 2097152 bytes
$cbr0 set packetSize_ 2097152
$cbr0 set maxpkts_ 1

if {$protocolo == "udp"} {
	#udp0
	set udp0 [new Agent/UDP]
	$ns attach-agent $n0 $udp0
	$cbr0 attach-agent $udp0

	#null0
	set null0 [new Agent/Null]
	$ns attach-agent $n7 $null0
	$ns connect $udp0 $null0

	#roteamento dinamico 
	$ns rtproto LS

	#cor
	$udp0 set class_ 1

} elseif {$protocolo == "tcp"} {
	#tcp
	set tcp [new Agent/TCP]
	$ns attach-agent $n0 $tcp
	$tcp set window_ $window
	$tcp set fid_ 1
	$cbr0 attach-agent $tcp	

	set sink [new Agent/TCPSink]			
	$ns attach-agent $n7 $sink
	$ns connect $tcp $sink

	#roteamento dinamico
	$ns rtproto LS

	#cor
	$tcp set class_ 1
}

#Cenario 2, trafego adicional
if {$opcao == "2"} {
 
#PC B -> PC D
	#udp1
	set udp1 [new Agent/UDP]
	$ns attach-agent $n1 $udp1
	$udp1 set class_ 2

	#cbr1
	set cbr1 [new Application/Traffic/CBR]
	$cbr1 set rate_ 6Mb
	$cbr1 attach-agent $udp1

	#null1
	set null1 [new Agent/Null]
	$ns attach-agent $n5 $null1
	$ns connect $udp1 $null1

#PC D -> PC C
	#udp2
	set udp2 [new Agent/UDP]
	$ns attach-agent $n5 $udp2
	$udp2 set class_ 3

	#cbr2
	set cbr2 [new Application/Traffic/CBR]
	$cbr2 set rate_ 5Mb
	$cbr2 attach-agent $udp2

	#null2
	set null2 [new Agent/Null]
	$ns attach-agent $n2 $null2
	$ns connect $udp2 $null2

	$ns at 0.5 "$cbr1 start"
	$ns at 0.5 "$cbr2 start"
	$ns at 2.5 "$cbr1 stop"
	$ns at 2.5 "$cbr2 stop"
}

#        Simulação falha        #
#$ns rtmodel-at 0.75 down $n2 $n5
#$ns rtmodel-at 0.9 up $n2 $n5


$ns at 0.5 "$cbr0 start"
$ns at 3.0 "$cbr0 stop"
$ns at 3.0 "fim"

$ns run
