require "formula"

class Ntopng-devel < Formula
  homepage "http://www.ntop.org"
  url 'https://svn.ntop.org/svn/ntop/trunk/ntopng/', :using => :svn, :tag => '7282'
 
  depends_on :autoconf
  depends_on :automake
  depends_on :libtool
  depends_on 'sqlite'
  depends_on 'pkg-config'
  depends_on 'lzlib'
  depends_on 'wget'
  depends_on 'libxml2'
  depends_on 'redis'
  depends_on 'glib'
  depends_on 'lbzip2'
  depends_on 'rrdtool'
  depends_on 'zeromq'
 
  def install
  
    system "./configure", "--prefix=#{prefix}"
    
    system "make" 
    system "make install" # if this fails, try separate make/make install steps
  end

  test do
   
    # The installed folder is not in the path, so use the entire path to any
    # executables being tested: `system "#{bin}/program", "do", "something"`.
    system "true"
  end
end
