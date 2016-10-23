#include<string>

class key_t {
    key_t(){};
    key_t(std::string s){key = s};
    std::string to_string(){return s};

private:
    std::string key;
}

class val_t {
    val_t(){};
    val_t(std::string s){val = s};
    std::string to_string(){return s};

private:
    std::string val;
}
