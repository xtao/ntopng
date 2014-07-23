require "formula"

class Ndpi < Formula
  homepage "http://www.ntop.org"
  url 'https://svn.ntop.org/svn/ntop/trunk/nDPI/'
  version "1.1.4"

  depends_on :autoconf => :build
  depends_on :automake => :build
  depends_on 'pkg-config' => :build
  depends_on :libtool => :build

  def install
    system "./configure", "--prefix=#{prefix}"
    system "make"
    system "make install"
  end

  test do
    # tests if it can load chisels
    `#{bin}/ndpiReader -h`
    assert_equal 0, $?.exitstatus
  end

end
