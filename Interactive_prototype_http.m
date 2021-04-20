s = struct('request_id', "1", 'get_beam_params', true);
body = matlab.net.http.MessageBody(jsonencode(s));
body.show;

contentTypeField = matlab.net.http.field.ContentTypeField('application/json');
header = [];
%%

post = matlab.net.http.RequestMethod.POST;
get = matlab.net.http.RequestMethod.GET;


%header = matlab.net.http.HeaderField('Content-Type','application/json');
ask_for_parameters = matlab.net.http.RequestMessage(post, header, body);
retrieve_parameters = matlab.net.http.RequestMessage(get, header);
show(ask_for_parameters);

[response, completedrequest, history] = send(ask_for_parameters, 'http://localhost:80/api/latest');
stat=response.StatusCode
%  [response, completedrequest, history] = send(retrieve_parameters, 'http://localhost:80/api/latest');
%  show(retrieve_parameters);
%  response.StatusCode


% https://www.mathworks.com/help/matlab/ref/matlab.net.http.requestmessage-class.html
% https://www.mathworks.com/help/matlab/ref/matlab.net.http.requestmessage.send.html#d123e607