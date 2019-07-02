Vagrant.configure('2') do |config|
  config.vm.box = 'ubuntu/bionic64'

  config.ssh.forward_agent = true

  config.vm.provider 'virtualbox' do |vb|
    vb.memory = '1024'
    vb.cpus = 1
  end

  config.vm.provision 'shell', privileged: true, inline: <<-SHELL
    add-apt-repository ppa:team-gcc-arm-embedded/ppa
    apt-get update

    apt-get install --yes git cmake ninja-build python libc++1 python-pip gcc-arm-embedded

    pip install nrfutil pyyaml

    # make the bash go into /vagrant dir when logging in
    if ! cat /home/vagrant/.bash_profile > /dev/null 2>&1 | grep 'cd /vagrant' > /dev/null 2>&1; then
      touch /home/vagrant/.bash_profile || echo .bash_profile exists
      echo 'cd /vagrant' >> /home/vagrant/.bash_profile
    fi
  SHELL

end
