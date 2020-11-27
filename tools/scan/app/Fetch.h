#pragma once

// Fetch one description file at a time to avoid swamping the TCP stack
struct Fetch {
	UPnP::Urn::Kind kind{};
	String url;
	String root;
	String path;
	unsigned attempts{0};

	Fetch() = default;
	Fetch(const Fetch&) = default;

	Fetch(UPnP::Urn::Kind kind, const String& url, const String& root, const String& path)
		: kind(kind), url(url), root(root), path(path)
	{
	}

	Fetch(const UPnP::Urn& urn) : kind(urn.kind), path(urn.toString())
	{
	}

	explicit operator bool() const
	{
		return kind != UPnP::Urn::Kind::none;
	}

	bool operator==(const Fetch& other)
	{
		return path == other.path;
	}

	UPnP::Urn urn() const
	{
		return UPnP::Urn(path);
	}

	String fullPath() const
	{
		String s;
		s += root;
		s += path;
		auto ext = path.lastIndexOf('.');
		auto sep = path.lastIndexOf('/');
		if(sep > ext) {
			s += ".xml";
		}
		return s;
	}

	String toString() const
	{
		String s;
		s += '(';
		s += ::toString(kind);
		s += ") ";
		if(url) {
			s += url;
			s += F("' -> '");
			s += root;
			s += path;
			s += '\'';
		} else {
			s += path;
		}
		return s;
	}
};

class FetchList
{
public:
	FetchList(const String& name) : name(name)
	{
	}

	bool contains(const Fetch& f)
	{
		return queue.contains(f) || done.contains(f);
	}

	bool isDone(const Fetch& f)
	{
		return done.contains(f);
	}

	bool add(Fetch f)
	{
		if(f.kind > UPnP::Urn::Kind::service) {
			f.kind = UPnP::Urn::Kind::none;
		}

		if(contains(f)) {
			return false;
		}

		if(f.kind == UPnP::Urn::Kind::service) {
			queue.insertElementAt(f, 0);
		} else {
			queue.add(f);
		}

		Serial.print(_F("Queuing '"));
		Serial.println(f.toString());

		return true;
	}

	Fetch& peek()
	{
		if(queue.count() == 0) {
			static Fetch nil;
			nil = Fetch();
			return nil;
		} else {
			return queue[0];
		}
	}

	Fetch pop()
	{
		if(queue.count() == 0) {
			return Fetch{};
		}

		Fetch f = queue.at(0);
		queue.remove(0);
		return f;
	}

	void finished(const Fetch& f)
	{
		done.add(f);
		queue.removeElement(f);
	}

	void finished(const String& url)
	{
		Fetch f;
		f.url = url;
		finished(f);
	}

	unsigned count() const
	{
		return queue.count();
	}

	String toString() const
	{
		String s;
		s += name;
		s += F(": fetched ");
		s += done.count();
		s += F(" of ");
		s += done.count() + queue.count();
		return s;
	}

private:
	using List = Vector<Fetch>;
	String name;
	List queue;
	List done;
};
